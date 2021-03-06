#ifndef BPT_GA_HPP
#define BPT_GA_HPP

#include <cstdlib>

#include <EASTL/vector.h>

#include <corex/math.hpp>

#include <bpt/ds.hpp>
#include <bpt/SelectionType.hpp>

namespace bpt
{
  class GA
  {
  public:
    GA();
    eastl::vector<eastl::vector<Solution>> generateSolutions(
      const eastl::vector<InputBuilding>& inputBuildings,
      const cx::NPolygon& boundingArea,
      const eastl::vector<eastl::vector<float>>& flowRates,
      eastl::vector<cx::NPolygon>& floodProneAreas,
      eastl::vector<cx::NPolygon>& landslideProneAreas,
      const float mutationRate,
      const int populationSize,
      const int numGenerations,
      const int tournamentSize,
      const int numPrevGenOffsprings,
      const float floodProneAreaPenalty,
      const float landslideProneAreaPenalty,
      const float buildingDistanceWeight,
      const bool isLocalSearchEnabled,
      const SelectionType selectionType);
    double getSolutionFitness(
      const Solution& solution,
      const eastl::vector<InputBuilding>& inputBuildings,
      const eastl::vector<eastl::vector<float>>& flowRates,
      const eastl::vector<cx::NPolygon>& floodProneAreas,
      const eastl::vector<cx::NPolygon>& landslideProneAreas,
      const float floodProneAreaPenalty,
      const float landslideProneAreaPenalty,
      const float buildingDistanceWeight);
    int getCurrentRunGenerationNumber();
    eastl::vector<float> getRecentRunAverageFitnesses();
    eastl::vector<float> getRecentRunBestFitnesses();
    eastl::vector<float> getRecentRunWorstFitnesses();
  private:
    eastl::array<Solution, 2> selectParents(
      const eastl::vector<Solution>& population,
      const int& tournamentSize,
      const SelectionType& selectionType);
    eastl::array<Solution, 2> runRouletteWheelSelection(
      const eastl::vector<Solution>& population);
    eastl::array<Solution, 2> runTournamentSelection(
      const eastl::vector<Solution>& population,
      const int& tournamentSize);
    void makeTwoParentsBreed(
      const Solution& parentA,
      const Solution& parentB,
      eastl::vector<Solution>& offsprings,
      int& numOffsprings,
      const int numOffspringsToMake,
      const float mutationRate,
      const cx::NPolygon& boundingArea,
      const eastl::vector<InputBuilding>& inputBuildings,
      const eastl::vector<eastl::vector<float>>& flowRates,
      const eastl::vector<cx::NPolygon>& floodProneAreas,
      const eastl::vector<cx::NPolygon>& landslideProneAreas,
      const float floodProneAreaPenalty,
      const float landslideProneAreaPenalty,
      const float buildingDistanceWeight);
    Solution
    generateRandomSolution(const eastl::vector<InputBuilding>& inputBuildings,
                           const cx::NPolygon& boundingArea);
    eastl::array<Solution, 2>
    crossoverSolutions(const Solution& solutionA,
                       const Solution& solutionB,
                       const cx::NPolygon& boundingArea,
                       const eastl::vector<InputBuilding>& inputBuildings);
    void mutateSolution(Solution& solution,
                        const cx::NPolygon& boundingArea,
                        const eastl::vector<InputBuilding>& inputBuildings);
    void applyBuddyBuddyMutation(
        Solution& solution,
        const cx::NPolygon& boundingArea,
        const eastl::vector<InputBuilding>& inputBuildings);
    void applyShakingMutation(
        Solution& solution,
        const cx::NPolygon& boundingArea,
        const eastl::vector<InputBuilding>& inputBuildings);
    void applyJiggleMutation(
        Solution& solution,
        const cx::NPolygon& boundingArea,
        const eastl::vector<InputBuilding>& inputBuildings);
    bool isSolutionFeasible(const Solution& solution,
                            const cx::NPolygon& boundingArea,
                            const eastl::vector<InputBuilding>& inputBuildings);
    bool doesSolutionHaveNoBuildingsOverlapping(
        const Solution& solution,
        const eastl::vector<InputBuilding>& inputBuildings);
    bool areSolutionBuildingsWithinBounds(
        const Solution& solution,
        const cx::NPolygon& boundingArea,
        const eastl::vector<InputBuilding>& inputBuildings);
    int currRunGenerationNumber;
    eastl::vector<float> recentRunAvgFitnesses;
    eastl::vector<float> recentRunBestFitnesses;
    eastl::vector<float> recentRunWorstFitnesses;
  };
}

#endif
