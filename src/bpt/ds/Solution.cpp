#include <cassert>

#include <corex/math.hpp>

#include <bpt/ds/Solution.hpp>

namespace bpt
{
  Solution::Solution()
      : genes()
      , numBuildings(0)
      , fitness(0)
      , hasFitnessSet(false) {}

  Solution::Solution(int numBuildings)
      : genes(numBuildings * 3, 0.f)
      , numBuildings(numBuildings)
      , fitness(0)
      , hasFitnessSet(false) {}

  Solution::Solution(const Solution& other) = default;

  void Solution::setBuildingXPos(int buildingIndex, float xPos)
  {
    this->genes[(buildingIndex * 3)] = xPos;
  }

  void Solution::setBuildingYPos(int buildingIndex, float yPos)
  {
    this->genes[(buildingIndex * 3) + 1] = yPos;
  }

  void Solution::setBuildingRotation(int buildingIndex, float rotation)
  {
    this->genes[(buildingIndex * 3) + 2] = rotation;
  }

  void Solution::setFitness(double fitness)
  {
    this->fitness = fitness;
    this->hasFitnessSet = true;
  }

  float Solution::getBuildingXPos(int buildingIndex) const
  {
    return this->genes[(buildingIndex * 3)];
  }

  float Solution::getBuildingYPos(int buildingIndex) const
  {
    return this->genes[(buildingIndex * 3) + 1];
  }

  float Solution::getBuildingRotation(int buildingIndex) const
  {
    return this->genes[(buildingIndex * 3) + 2];
  }

  int Solution::getNumBuildings() const
  {
    return this->numBuildings;
  }

  double Solution::getFitness() const
  {
    assert(this->hasFitnessSet);
    return this->fitness;
  }

  bool Solution::operator==(const Solution& other)
  {
    if (this->genes.size() == other.genes.size()) {
      for (int i = 0; i < this->genes.size(); i++) {
        if (!corex::core::floatEquals(this->genes[i], other.genes[i])) {
          return false;
        }
      }

      return true;
    }

    return false;
  }

  bool Solution::operator!=(const Solution& other)
  {
    return !((*this) == other);
  }
}