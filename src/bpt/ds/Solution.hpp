#ifndef BPT_DS_SOLUTION_HPP
#define BPT_DS_SOLUTION_HPP

#include <cstdlib>

#include <EASTL/vector.h>

namespace bpt
{
  class Solution
  {
    // Solution representation:
    //   [ xPos of building 0, yPos of building 0, rotation of building 0, ... ]
  public:
    Solution();
    Solution(const Solution& other);
    Solution(int numBuildings);

    void setBuildingXPos(int buildingIndex, float xPos);
    void setBuildingYPos(int buildingIndex, float yPos);
    void setBuildingRotation(int buildingIndex, float rotation);
    void setFitness(double fitness);
    float getBuildingXPos(int buildingIndex) const;
    float getBuildingYPos(int buildingIndex) const;
    float getBuildingRotation(int buildingIndex) const;
    int getNumBuildings() const;
    double getFitness() const;

    bool operator==(const Solution& other);
    bool operator!=(const Solution& other);
  private:
    eastl::vector<float> genes;
    int numBuildings;
    double fitness;
    bool hasFitnessSet;
  };
}

#endif
