/**
 * @file print_input_processing.hpp
 * @author Ryan Curtin
 *
 * Print input processing for a Python binding option.
 */
#ifndef MLPACK_BINDINGS_PYTHON_PRINT_INPUT_PROCESSING_HPP
#define MLPACK_BINDINGS_PYTHON_PRINT_INPUT_PROCESSING_HPP

#include <mlpack/prereqs.hpp>
#include "get_arma_type.hpp"
#include "get_numpy_type.hpp"
#include "get_numpy_type_char.hpp"
#include "get_cython_type.hpp"
#include "get_python_type.hpp"
#include "strip_type.hpp"

namespace mlpack {
namespace bindings {
namespace python {

/**
 * Print input processing for a standard option type.
 */
template<typename T>
void PrintInputProcessing(
    const util::ParamData& d,
    const size_t indent,
    const typename boost::disable_if<arma::is_arma_type<T>>::type* = 0,
    const typename boost::disable_if<data::HasSerialize<T>>::type* = 0,
    const typename boost::disable_if<std::is_same<T,
        std::tuple<data::DatasetInfo, arma::mat>>>::type* = 0)
{
  // The copy_all_inputs parameter must be handled first, and therefore is
  // outside the scope of this code.
  if (d.name == "copy_all_inputs")
    return;

  const std::string prefix(indent, ' ');

  std::string def = "None";
  if (std::is_same<T, bool>::value)
    def = "False";

  // Make sure that we don't use names that are Python keywords.
  std::string name = (d.name == "lambda") ? "lambda_" : d.name;

  /**
   * This gives us code like:
   *
   * # Detect if the parameter was passed; set if so.
   * if param_name is not None:
   *   SetParam[int](<const string> 'param_name', param_name)
   *   CLI.SetPassed(<const string> 'param_name')
   */
  std::cout << prefix << "# Detect if the parameter was passed; set if so."
      << std::endl;
  if (!d.required)
  {
    std::cout << prefix << "if " << name << " is not " << def << ":"
        << std::endl;

    std::cout << prefix << "  SetParam[" << GetCythonType<T>(d) << "](<const "
        << "string> '" << d.name << "', ";
    if (GetCythonType<T>(d) == "string")
      std::cout << name << ".encode(\"UTF-8\")";
    else if (GetCythonType<T>(d) == "vector[string]")
      std::cout << "[i.encode(\"UTF-8\") for i in " << name << "]";
    else
      std::cout << name;
    std::cout << ")" << std::endl;
    std::cout << prefix << "  CLI.SetPassed(<const string> '" << d.name
        << "')" << std::endl;

    // If this parameter is "verbose", then enable verbose output.
    if (d.name == "verbose")
      std::cout << prefix << "  EnableVerbose()" << std::endl;
  }
  else
  {
    std::cout << prefix << "SetParam[" << GetCythonType<T>(d) << "](<const "
        << "string> '" << d.name << "', ";
    if (GetCythonType<T>(d) == "string")
      std::cout << name << ".encode(\"UTF-8\")";
    else if (GetCythonType<T>(d) == "vector[string]")
      std::cout << "[i.encode(\"UTF-8\") for i in " << name << "]";
    else
      std::cout << name;
    std::cout << ")" << std::endl;
    std::cout << prefix << "CLI.SetPassed(<const string> '" << d.name << "')"
        << std::endl;
  }
  std::cout << std::endl; // Extra line is to clear up the code a bit.
}

/**
 * Print input processing for a matrix type.
 */
template<typename T>
void PrintInputProcessing(
    const util::ParamData& d,
    const size_t indent,
    const typename boost::enable_if<arma::is_arma_type<T>>::type* = 0)
{
  const std::string prefix(indent, ' ');

  /**
   * This gives us code like:
   *
   * # Detect if the parameter was passed; set if so.
   * if param_name is not None:
   *   param_name_tuple = to_matrix(param_name)
   *   param_name_mat = arma_numpy.numpy_to_mat_d(param_name_tuple[0],
   *       param_name_tuple[1])
   *   SetParam[mat](<const string> 'param_name', dereference(param_name_mat))
   *   CLI.SetPassed(<const string> 'param_name')
   */
  std::cout << prefix << "# Detect if the parameter was passed; set if so."
      << std::endl;
  if (!d.required)
  {
    std::cout << prefix << "if " << d.name << " is not None:" << std::endl;

    std::cout << prefix << "  " << d.name << "_tuple = to_matrix(" << d.name
        << ", dtype=" << GetNumpyType<typename T::elem_type>() << ", "
        << "copy=CLI.HasParam('copy_all_inputs'))" << std::endl;
    std::cout << prefix << "  " << d.name << "_mat = arma_numpy.numpy_to_"
        << GetArmaType<T>() << "_" << GetNumpyTypeChar<T>() << "(" << d.name
        << "_tuple[0], " << d.name << "_tuple[1])" << std::endl;
    std::cout << prefix << "  SetParam[" << GetCythonType<T>(d) << "](<const "
        << "string> '" << d.name << "', dereference(" << d.name << "_mat))"
        << std::endl;
    std::cout << prefix << "  CLI.SetPassed(<const string> '" << d.name << "')"
        << std::endl;
    std::cout << prefix << "  del " << d.name << "_mat";
  }
  else
  {
    std::cout << prefix << d.name << "_tuple = to_matrix(" << d.name
        << ", dtype=" << GetNumpyType<typename T::elem_type>() << ", "
        << "copy=CLI.HasParam('copy_all_inputs'))" << std::endl;
    std::cout << prefix << d.name << "_mat = arma_numpy.numpy_to_"
        << GetArmaType<T>() << "_" << GetNumpyTypeChar<T>() << "(" << d.name
        << "_tuple[0], " << d.name << "_tuple[1])" << std::endl;
    std::cout << prefix << "SetParam[" << GetCythonType<T>(d) << "](<const "
        << "string> '" << d.name << "', dereference(" << d.name << "_mat))"
        << std::endl;
    std::cout << prefix << "CLI.SetPassed(<const string> '" << d.name << "')"
        << std::endl;
    std::cout << prefix << "del " << d.name << "_mat";
  }
  std::cout << std::endl;
}

/**
 * Print input processing for a serializable type.
 */
template<typename T>
void PrintInputProcessing(
    const util::ParamData& d,
    const size_t indent,
    const typename boost::disable_if<arma::is_arma_type<T>>::type* = 0,
    const typename boost::enable_if<data::HasSerialize<T>>::type* = 0)
{
  // First, get the correct class name if needed.
  std::string strippedType, printedType, defaultsType;
  StripType(d.cppType, strippedType, printedType, defaultsType);

  const std::string prefix(indent, ' ');

  /**
   * This gives us code like:
   *
   * # Detect if the parameter was passed; set if so.
   * if param_name is not None:
   *   try:
   *     SetParamPtr[Model]('param_name', (<ModelType?> param_name).modelptr,
   *         CLI.HasParam('copy_all_inputs'))
   *   except TypeError as e:
   *     if type(param_name).__name__ == "ModelType":
   *       SetParamPtr[Model]('param_name', (<ModelType> param_name).modelptr,
   *           CLI.HasParam('copy_all_inputs'))
   *     else:
   *       raise e
   *   CLI.SetPassed(<const string> 'param_name')
   */
  std::cout << prefix << "# Detect if the parameter was passed; set if so."
      << std::endl;
  if (!d.required)
  {
    std::cout << prefix << "if " << d.name << " is not None:" << std::endl;
    std::cout << prefix << "  try:" << std::endl;
    std::cout << prefix << "    SetParamPtr[" << strippedType << "]('" << d.name
        << "', (<" << strippedType << "Type?> " << d.name << ").modelptr, "
        << "CLI.HasParam('copy_all_inputs'))" << std::endl;
    std::cout << prefix << "  except TypeError as e:" << std::endl;
    std::cout << prefix << "    if type(" << d.name << ").__name__ == '"
        << strippedType << "Type':" << std::endl;
    std::cout << prefix << "      SetParamPtr[" << strippedType << "]('"
        << d.name << "', (<" << strippedType << "Type> " << d.name
        << ").modelptr, CLI.HasParam('copy_all_inputs'))" << std::endl;
    std::cout << prefix << "    else:" << std::endl;
    std::cout << prefix << "      raise e" << std::endl;
    std::cout << prefix << "  CLI.SetPassed(<const string> '" << d.name << "')"
        << std::endl;
  }
  else
  {
    std::cout << prefix << "try:" << std::endl;
    std::cout << prefix << "  SetParamPtr[" << strippedType << "]('" << d.name
        << "', (<" << strippedType << "Type?> " << d.name << ").modelptr, "
        << "CLI.HasParam('copy_all_inputs'))" << std::endl;
    std::cout << prefix << "except TypeError as e:" << std::endl;
    std::cout << prefix << "  if type(" << d.name << ").__name__ == '"
        << strippedType << "Type':" << std::endl;
    std::cout << prefix << "    SetParamPtr[" << strippedType << "]('" << d.name
        << "', (<" << strippedType << "Type> " << d.name << ").modelptr, "
        << "CLI.HasParam('copy_all_inputs'))" << std::endl;
    std::cout << prefix << "  else:" << std::endl;
    std::cout << prefix << "    raise e" << std::endl;
    std::cout << prefix << "CLI.SetPassed(<const string> '" << d.name << "')"
        << std::endl;
  }
  std::cout << std::endl;
}

/**
 * Print input processing for a matrix/DatasetInfo type.
 */
template<typename T>
void PrintInputProcessing(
    const util::ParamData& d,
    const size_t indent,
    const typename boost::enable_if<std::is_same<T,
        std::tuple<data::DatasetInfo, arma::mat>>>::type* = 0)
{
  // The user should pass in a matrix type of some sort.
  const std::string prefix(indent, ' ');

  /** We want to generate code like the following:
   *
   * if param_name is not None:
   *   param_name_tuple = to_matrix_with_info(param_name)
   *   param_name_mat = arma_numpy.numpy_to_matrix_d(param_name_tuple[0])
   *   SetParamWithInfo[mat](<const string> 'param_name',
   *       dereference(param_name_mat), &param_name_tuple[1][0])
   *   CLI.SetPassed(<const string> 'param_name')
   */
  std::cout << prefix << "cdef np.ndarray " << d.name << "_dims" << std::endl;
  std::cout << prefix << "# Detect if the parameter was passed; set if so."
      << std::endl;
  if (!d.required)
  {
    std::cout << prefix << "if " << d.name << " is not None:" << std::endl;
    std::cout << prefix << "  " << d.name << "_tuple = to_matrix_with_info("
        << d.name << ", dtype=np.double, copy=CLI.HasParam('copy_all_inputs'))"
        << std::endl;
    std::cout << prefix << "  " << d.name << "_mat = arma_numpy.numpy_to_mat_d("
        << d.name << "_tuple[0], " << d.name << "_tuple[1])" << std::endl;
    std::cout << prefix << "  " << d.name << "_dims = " << d.name << "_tuple[2]"
        << std::endl;
    std::cout << prefix << "  SetParamWithInfo[arma.Mat[double]](<const string>"
        << " '" << d.name << "', dereference(" << d.name << "_mat), <const "
        << "bool*> " << d.name << "_dims.data)" << std::endl;
    std::cout << prefix << "  CLI.SetPassed(<const string> '" << d.name << "')"
        << std::endl;
    std::cout << prefix << "  del " << d.name << "_mat" << std::endl;
  }
  else
  {
    std::cout << prefix << d.name << "_tuple = to_matrix_with_info(" << d.name
        << ", dtype=np.double, copy=CLI.HasParam('copy_all_inputs'))"
        << std::endl;
    std::cout << prefix << d.name << "_mat = arma_numpy.numpy_to_mat_d("
        << d.name << "_tuple[0], " << d.name << "_tuple[1])" << std::endl;
    std::cout << prefix << d.name << "_dims = " << d.name << "_tuple[2]"
        << std::endl;
    std::cout << prefix << "SetParamWithInfo[arma.Mat[double]](<const string>"
        << " '" << d.name << "', dereference(" << d.name << "_mat), <const "
        << "bool*> " << d.name << "_dims.data)" << std::endl;
    std::cout << prefix << "CLI.SetPassed(<const string> '" << d.name << "')"
        << std::endl;
    std::cout << prefix << "del " << d.name << "_mat" << std::endl;
  }
  std::cout << std::endl;
}

/**
 * Given parameter information and the current number of spaces for indentation,
 * print the code to process the input to cout.  This code assumes that
 * data.input is true, and should not be called when data.input is false.
 *
 * The number of spaces to indent should be passed through the input pointer.
 *
 * @param d Parameter data struct.
 * @param input Pointer to size_t holding the indentation.
 * @param output Unused parameter.
 */
template<typename T>
void PrintInputProcessing(const util::ParamData& d,
                          const void* input,
                          void* /* output */)
{
  PrintInputProcessing<typename std::remove_pointer<T>::type>(d,
      *((size_t*) input));
}

} // namespace python
} // namespace bindings
} // namespace mlpack

#endif
