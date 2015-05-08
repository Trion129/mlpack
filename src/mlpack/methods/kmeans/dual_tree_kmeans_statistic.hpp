/**
 * @file dtnn_statistic.hpp
 * @author Ryan Curtin
 *
 * Statistic for dual-tree nearest neighbor search based k-means clustering.
 */
#ifndef __MLPACK_METHODS_KMEANS_DTNN_STATISTIC_HPP
#define __MLPACK_METHODS_KMEANS_DTNN_STATISTIC_HPP

#include <mlpack/methods/neighbor_search/neighbor_search_stat.hpp>

namespace mlpack {
namespace kmeans {

class DualTreeKMeansStatistic : public
    neighbor::NeighborSearchStat<neighbor::NearestNeighborSort>
{
 public:
  DualTreeKMeansStatistic() :
      neighbor::NeighborSearchStat<neighbor::NearestNeighborSort>(),
      upperBound(DBL_MAX),
      lowerBound(DBL_MAX),
      owner(size_t(-1)),
      pruned(size_t(-1)),
      staticPruned(false),
      staticUpperBoundMovement(0.0),
      staticLowerBoundMovement(0.0),
      centroid(),
      trueParent(NULL)
  {
    // Nothing to do.
  }

  template<typename TreeType>
  DualTreeKMeansStatistic(TreeType& node) :
      neighbor::NeighborSearchStat<neighbor::NearestNeighborSort>(),
      upperBound(DBL_MAX),
      lowerBound(DBL_MAX),
      owner(size_t(-1)),
      pruned(size_t(-1)),
      staticPruned(false),
      staticUpperBoundMovement(0.0),
      staticLowerBoundMovement(0.0),
      trueParent(node.Parent())
  {
    // Empirically calculate the centroid.
    centroid.zeros(node.Dataset().n_rows);
    for (size_t i = 0; i < node.NumPoints(); ++i)
      centroid += node.Dataset().col(node.Point(i));

    for (size_t i = 0; i < node.NumChildren(); ++i)
      centroid += node.Child(i).NumDescendants() *
          node.Child(i).Stat().Centroid();

    centroid /= node.NumDescendants();

    // Set the true children correctly.
    trueChildren.resize(node.NumChildren());
    for (size_t i = 0; i < node.NumChildren(); ++i)
      trueChildren[i] = &node.Child(i);
  }

  double UpperBound() const { return upperBound; }
  double& UpperBound() { return upperBound; }

  double LowerBound() const { return lowerBound; }
  double& LowerBound() { return lowerBound; }

  const arma::vec& Centroid() const { return centroid; }
  arma::vec& Centroid() { return centroid; }

  size_t Owner() const { return owner; }
  size_t& Owner() { return owner; }

  size_t Pruned() const { return pruned; }
  size_t& Pruned() { return pruned; }

  bool StaticPruned() const { return staticPruned; }
  bool& StaticPruned() { return staticPruned; }

  double StaticUpperBoundMovement() const { return staticUpperBoundMovement; }
  double& StaticUpperBoundMovement() { return staticUpperBoundMovement; }

  double StaticLowerBoundMovement() const { return staticLowerBoundMovement; }
  double& StaticLowerBoundMovement() { return staticLowerBoundMovement; }

  void* TrueParent() const { return trueParent; }
  void*& TrueParent() { return trueParent; }

  void* TrueChild(const size_t i) const { return trueChildren[i]; }
  void*& TrueChild(const size_t i) { return trueChildren[i]; }

  std::string ToString() const
  {
    std::ostringstream o;
    o << "DualTreeKMeansStatistic [" << this << "]:\n";
    o << "  Upper bound: " << upperBound << ".\n";
    o << "  Lower bound: " << lowerBound << ".\n";
    o << "  Pruned: " << pruned << ".\n";
    o << "  Static pruned: " << staticPruned << ".\n";
    o << "  Owner: " << owner << ".\n";
    return o.str();
  }

 private:
  double upperBound;
  double lowerBound;
  size_t owner;
  size_t pruned;
  bool staticPruned;
  double staticUpperBoundMovement;
  double staticLowerBoundMovement;
  arma::vec centroid;
  void* trueParent;
  std::vector<void*> trueChildren;
};

} // namespace kmeans
} // namespace mlpack

#endif
