#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <random>

#include <EASTL/array.h>
#include <EASTL/functional.h>
#include <EASTL/vector.h>

#include <corex/math.hpp>
#include <corex/utils.hpp>

#include <bpt/ds.hpp>
#include <bpt/GA.hpp>

namespace bpt
{
  GA::GA()
      : currRunGenerationNumber(-1)
      , recentRunAvgFitnesses()
      , recentRunBestFitnesses()
      , recentRunWorstFitnesses() {}

  eastl::vector<eastl::vector<Solution>> GA::generateSolutions(
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
      const SelectionType selectionType)
  {
    assert(flowRates.size() == inputBuildings.size());

    eastl::vector<eastl::vector<Solution>> solutions;
    eastl::vector<Solution> population(populationSize);

    this->recentRunAvgFitnesses.clear();
    this->recentRunBestFitnesses.clear();
    this->recentRunWorstFitnesses.clear();

    std::cout << "|| Generating Initial Population..." << std::endl;
    for (int i = 0; i < populationSize; i++) {
      std::cout << "Generating solution #" << i << "..." << std::endl;
      population[i] = this->generateRandomSolution(inputBuildings,
                                                   boundingArea);
      population[i].setFitness(
          this->getSolutionFitness(
              population[i],
              inputBuildings,
              flowRates,
              floodProneAreas,
              landslideProneAreas,
              floodProneAreaPenalty,
              landslideProneAreaPenalty,
              buildingDistanceWeight));
    }

    // Add the initial population.
    solutions.push_back(population);

    Solution bestSolution = *std::min_element(
        population.begin(),
        population.end(),
        [](Solution solutionA, Solution solutionB) {
          return cx::floatLessThan(solutionA.getFitness(),
                                            solutionB.getFitness());
        }
    );
    Solution worstSolution = *std::max_element(
        population.begin(),
        population.end(),
        [](Solution solutionA, Solution solutionB) {
          return cx::floatLessThan(solutionA.getFitness(),
                                            solutionB.getFitness());
        }
    );

    // Add statistics about the initial population.
    double fitnessAverage = 0.0;
    for (Solution& sol : population) {
      fitnessAverage += sol.getFitness();
    }

    fitnessAverage = fitnessAverage / population.size();
    this->recentRunAvgFitnesses.push_back(static_cast<float>(fitnessAverage));

    this->recentRunBestFitnesses.push_back(static_cast<float>(
                                               bestSolution.getFitness()));

    this->recentRunWorstFitnesses.push_back(static_cast<float>(
                                                worstSolution.getFitness()));

    const int numOffspringsToMake = populationSize - numPrevGenOffsprings;
    for (int i = 0; i < numGenerations; i++) {
      this->currRunGenerationNumber++;

      int numOffsprings = 0;
      eastl::vector<Solution> newOffsprings(numOffspringsToMake);
      while (numOffsprings < numOffspringsToMake) {
        // Standard Tournament Selection.
        auto parents = this->selectParents(population,
                                           tournamentSize,
                                           selectionType);
        Solution parentA = parents[0];
        Solution parentB = parents[1];

        // Make sure we have individuals from the population, and not just
        // empty solutions.
        assert(parentA.getNumBuildings() != 0);
        assert(parentB.getNumBuildings() != 0);

        // Breeding time.
        this->makeTwoParentsBreed(parentA,
                                  parentB,
                                  newOffsprings,
                                  numOffsprings,
                                  numOffspringsToMake,
                                  mutationRate,
                                  boundingArea,
                                  inputBuildings,
                                  flowRates,
                                  floodProneAreas,
                                  landslideProneAreas,
                                  floodProneAreaPenalty,
                                  landslideProneAreaPenalty,
                                  buildingDistanceWeight);
      }

      std::sort(
          population.begin(),
          population.end(),
          [](Solution& solutionA, Solution& solutionB) {
            return cx::floatLessThan(solutionA.getFitness(),
                                              solutionB.getFitness());
          }
      );

      // Keep only a set number of offsprings from the previous generation.
      for (int i = numPrevGenOffsprings; i < population.size(); i++) {
        population[i] = newOffsprings[i - numPrevGenOffsprings];
      }

      std::sort(
          population.begin(),
          population.end(),
          [](Solution& solutionA, Solution& solutionB) {
            return cx::floatLessThan(solutionA.getFitness(),
                                              solutionB.getFitness());
          }
      );

      bestSolution = population[0];

      // Might add the local search feature in the future.

      bestSolution.setFitness(this->getSolutionFitness(
          bestSolution,
          inputBuildings,
          flowRates,
          floodProneAreas,
          landslideProneAreas,
          floodProneAreaPenalty,
          landslideProneAreaPenalty,
          buildingDistanceWeight));

      solutions.push_back(population);

      double fitnessAverage = 0.0;
      for (Solution& sol : population) {
        fitnessAverage += sol.getFitness();
      }

      fitnessAverage = fitnessAverage / population.size();
      this->recentRunAvgFitnesses.push_back(static_cast<float>(fitnessAverage));

      this->recentRunBestFitnesses.push_back(static_cast<float>(
                                                 bestSolution.getFitness()));

      worstSolution = population.back();

      this->recentRunWorstFitnesses.push_back(static_cast<float>(
                                                  worstSolution.getFitness()));
    }

    this->currRunGenerationNumber = -1;

    return solutions;
  }

  double GA::getSolutionFitness(
      const Solution& solution,
      const eastl::vector<InputBuilding>& inputBuildings,
      const eastl::vector<eastl::vector<float>>& flowRates,
      const eastl::vector<cx::NPolygon>& floodProneAreas,
      const eastl::vector<cx::NPolygon>& landslideProneAreas,
      const float floodProneAreaPenalty,
      const float landslideProneAreaPenalty,
      const float buildingDistanceWeight)
  {
    double fitness = 0.0;

    // Compute fitness for the inter-building distance part.
    for (int i = 0; i < solution.getNumBuildings(); i++) {
      assert(flowRates[i].size() == solution.getNumBuildings());
      for (int j = 1; j < solution.getNumBuildings(); j++) {
        if (i == j) {
          continue;
        }

        fitness += static_cast<double>(
            cx::distance2D(cx::Point{
                                        solution.getBuildingXPos(i),
                                        solution.getBuildingYPos(i)
                                    },
                                    cx::Point{
                                        solution.getBuildingXPos(j),
                                        solution.getBuildingYPos(j)
                                    })
            * flowRates[i][j]
        );
      }
    }

    fitness *= buildingDistanceWeight;

    // Compute penalty for placing buildings in hazard areas.
    for (int i = 0; i < solution.getNumBuildings(); i++) {
      cx::Rectangle building {
          solution.getBuildingXPos(i),
          solution.getBuildingYPos(i),
          inputBuildings[i].width,
          inputBuildings[i].length,
          solution.getBuildingRotation(i)
      };
      // Compute penalty for placing a building in a flood-prone area.
      for (const cx::NPolygon& area : floodProneAreas) {
        if (cx::isRectIntersectingNPolygon(building, area)) {
          fitness += floodProneAreaPenalty;
        }
      }

      // Compute penalty for placing a building in a landslide-prone area.
      for (const cx::NPolygon& area : landslideProneAreas) {
        if (cx::isRectIntersectingNPolygon(building, area)) {
          fitness += landslideProneAreaPenalty;
        }
      }
    }

    return fitness;
  }

  int GA::getCurrentRunGenerationNumber()
  {
    return this->currRunGenerationNumber;
  }

  eastl::vector<float> GA::getRecentRunAverageFitnesses()
  {
    return this->recentRunAvgFitnesses;
  }

  eastl::vector<float> GA::getRecentRunBestFitnesses()
  {
    return this->recentRunBestFitnesses;
  }

  eastl::vector<float> GA::getRecentRunWorstFitnesses()
  {
    return this->recentRunWorstFitnesses;
  }

  eastl::array<Solution, 2> GA::selectParents(
      const eastl::vector<Solution>& population,
      const int& tournamentSize,
      const SelectionType& selectionType)
  {
    eastl::array<Solution, 2> parents;
    switch (selectionType) {
      case SelectionType::RWS:
        parents = this->runRouletteWheelSelection(population);
      case SelectionType::TS:
        parents = this->runTournamentSelection(population, tournamentSize);
      default:
        break;
    }

    return parents;
  }

  eastl::array<Solution, 2> GA::runRouletteWheelSelection(
      const eastl::vector<Solution>& population)
  {
    // Let's try roulette wheel selection. Code based from:
    //   https://stackoverflow.com/a/26316267/1116098
    eastl::vector<double> popFitnesses;
    for (int i = 0; i < population.size(); i++) {
      popFitnesses.push_back(population[i].getFitness());
    }

    double fitnessSum = std::accumulate(popFitnesses.begin(),
                                        popFitnesses.end(),
                                        0);
    double maxFitness = *std::max_element(
        popFitnesses.begin(),
        popFitnesses.end(),
        [](double a, double b) {
          return cx::floatLessEqual(a, b);
        });
    double minFitness = *std::min_element(
        popFitnesses.begin(),
        popFitnesses.end(),
        [](double a, double b) {
          return cx::floatLessEqual(a, b);
        });
    double upperBound = maxFitness + minFitness;

    std::uniform_real_distribution<double> fitnessDistrib {0, fitnessSum };

    eastl::array<Solution, 2> parents;
    for (int i = 0; i < parents.size(); i++) {
      double p = cx::generateRandomReal(fitnessDistrib);
      parents[i] = population[0]; // Default selection.
      for (int j = 0; j < popFitnesses.size(); j++) {
        p -= upperBound - popFitnesses[i];

        if (cx::floatLessEqual(p, 0.f)) {
          parents[i] = population[j];
          break;
        }
      }
    }

    return parents;
  }

  eastl::array<Solution, 2> GA::runTournamentSelection(
      const eastl::vector<Solution>& population,
      const int& tournamentSize)
  {
    std::uniform_int_distribution<int> chromosomeDistribution{
        0, static_cast<int>(population.size() - 1)
    };

    eastl::array<Solution, 2> parents;
    for (int j = 0; j < tournamentSize; j++) {
      int parentIndex = cx::generateRandomInt(
          chromosomeDistribution);
      if (j == 0 // Boolean short-circuit. Hehe.
          || population[parentIndex].getFitness() < parents[0].getFitness()) {
        parents[1] = parents[0];
        parents[0] = population[parentIndex];
      } else if (parents[1].getNumBuildings() == 0 // Boolean short again.
                 || population[parentIndex].getFitness()
                    < parents[1].getFitness()) {
        parents[1] = population[parentIndex];
      }
    }

    return parents;
  }

  void GA::makeTwoParentsBreed(
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
      const float buildingDistanceWeight)
  {
    std::uniform_real_distribution<float> mutationChanceDistribution{
        0.f, 1.f
    };
    auto children = this->crossoverSolutions(parentA,
                                             parentB,
                                             boundingArea,
                                             inputBuildings);
    offsprings[numOffsprings] = children[0];
    offsprings[numOffsprings].setFitness(this->getSolutionFitness(
        offsprings[numOffsprings],
        inputBuildings,
        flowRates,
        floodProneAreas,
        landslideProneAreas,
        floodProneAreaPenalty,
        landslideProneAreaPenalty,
        buildingDistanceWeight));

    // Mutation
    float mutationProbability = cx::generateRandomReal(
        mutationChanceDistribution);
    if (cx::floatLessThan(mutationProbability, mutationRate)) {
      this->mutateSolution(offsprings[numOffsprings],
                           boundingArea,
                           inputBuildings);
      offsprings[numOffsprings].setFitness(this->getSolutionFitness(
          offsprings[numOffsprings],
          inputBuildings,
          flowRates,
          floodProneAreas,
          landslideProneAreas,
          floodProneAreaPenalty,
          landslideProneAreaPenalty,
          buildingDistanceWeight));
    }

    numOffsprings++;

    // In cases where the population size is not an even number, a child
    // will have to be dropped. As such, we'll only add the second
    // generated child if it has a fitness better than the worst solution
    // in the new generation.
    if (numOffsprings == numOffspringsToMake) {
      auto weakestSolutionIter = std::max_element(
          offsprings.begin(),
          offsprings.end(),
          [](Solution solutionA, Solution solutionB) -> bool {
            return cx::floatLessThan(solutionA.getFitness(),
                                              solutionB.getFitness());
          }
      );

      if (cx::floatLessThan(children[1].getFitness(),
                                     weakestSolutionIter->getFitness())) {
        int weakestSolutionIndex = std::distance(offsprings.begin(),
                                                     weakestSolutionIter);
        offsprings[weakestSolutionIndex] = children[1];
        offsprings[weakestSolutionIndex].setFitness(
            this->getSolutionFitness(
                offsprings[weakestSolutionIndex],
                inputBuildings,
                flowRates,
                floodProneAreas,
                landslideProneAreas,
                floodProneAreaPenalty,
                landslideProneAreaPenalty,
                buildingDistanceWeight));

        float mutationProbability = cx::generateRandomReal(
            mutationChanceDistribution);
        if (cx::floatLessThan(mutationProbability, mutationRate)) {
          this->mutateSolution(offsprings[weakestSolutionIndex],
                               boundingArea,
                               inputBuildings);
          offsprings[weakestSolutionIndex].setFitness(
              this->getSolutionFitness(
                  offsprings[weakestSolutionIndex],
                  inputBuildings,
                  flowRates,
                  floodProneAreas,
                  landslideProneAreas,
                  floodProneAreaPenalty,
                  landslideProneAreaPenalty,
                  buildingDistanceWeight));
        }
      }
    } else {
      offsprings[numOffsprings] = children[1];
      offsprings[numOffsprings].setFitness(this->getSolutionFitness(
          offsprings[numOffsprings],
          inputBuildings,
          flowRates,
          floodProneAreas,
          landslideProneAreas,
          floodProneAreaPenalty,
          landslideProneAreaPenalty,
          buildingDistanceWeight));

      float mutationProbability = cx::generateRandomReal(
          mutationChanceDistribution);
      if (cx::floatLessThan(mutationProbability, mutationRate)) {
        this->mutateSolution(offsprings[numOffsprings],
                             boundingArea,
                             inputBuildings);
        offsprings[numOffsprings].setFitness(this->getSolutionFitness(
            offsprings[numOffsprings],
            inputBuildings,
            flowRates,
            floodProneAreas,
            landslideProneAreas,
            floodProneAreaPenalty,
            landslideProneAreaPenalty,
            buildingDistanceWeight));
      }

      numOffsprings++;
    }
  }

  Solution
  GA::generateRandomSolution(
      const eastl::vector<InputBuilding>& inputBuildings,
      const cx::NPolygon& boundingArea)
  {
    float minX = std::min_element(
        boundingArea.vertices.begin(),
        boundingArea.vertices.end(),
        [](cx::Point ptA, cx::Point ptB) -> bool {
          return ptA.x < ptB.x;
        }
    )->x;
    float maxX = std::max_element(
        boundingArea.vertices.begin(),
        boundingArea.vertices.end(),
        [](cx::Point ptA, cx::Point ptB) -> bool {
          return ptA.x < ptB.x;
        }
    )->x;
    float minY = std::min_element(
        boundingArea.vertices.begin(),
        boundingArea.vertices.end(),
        [](cx::Point ptA, cx::Point ptB) -> bool {
          return ptA.y < ptB.y;
        }
    )->y;
    float maxY = std::max_element(
        boundingArea.vertices.begin(),
        boundingArea.vertices.end(),
        [](cx::Point ptA, cx::Point ptB) -> bool {
          return ptA.y < ptB.y;
        }
    )->y;

    std::uniform_real_distribution<float> xPosDistribution{ minX, maxX };
    std::uniform_real_distribution<float> yPosDistribution{ minY, maxY };
    std::uniform_real_distribution<float> rotationDistribution{ 0.f, 360.f };

    Solution solution{ static_cast<int>(inputBuildings.size()) };
    do {
      for (int i = 0; i < inputBuildings.size(); i++) {
        cx::Point buildingPos { 0.f, 0.f };
        float buildingRotation = 0.f;
        cx::Rectangle buildingRect {
            buildingPos.x,
            buildingPos.y,
            inputBuildings[i].width,
            inputBuildings[i].length,
            buildingRotation
        };
        do {
          buildingPos.x = cx::generateRandomReal(xPosDistribution);
          buildingPos.y = cx::generateRandomReal(yPosDistribution);
          buildingRotation = cx::generateRandomReal(
              rotationDistribution);
          buildingRect.x = buildingPos.x;
          buildingRect.y = buildingPos.y;
          buildingRect.angle = buildingRotation;
        } while (!isRectWithinNPolygon(buildingRect, boundingArea));

        solution.setBuildingXPos(i, buildingPos.x);
        solution.setBuildingYPos(i, buildingPos.y);
        solution.setBuildingRotation(i, buildingRotation);
      }
    } while (!this->isSolutionFeasible(solution, boundingArea, inputBuildings));

    return solution;
  }

  eastl::array<Solution, 2>
  GA::crossoverSolutions(const Solution& solutionA,
                         const Solution& solutionB,
                         const cx::NPolygon& boundingArea,
                         const eastl::vector<InputBuilding>& inputBuildings)
  {
    // We're doing uniform crossover.
    std::uniform_int_distribution<int> parentDistrib{0, 1 };
    int numBuildings = solutionA.getNumBuildings();

    // Prevent unnecessary copying of the parents.
    eastl::array<const Solution* const, 2> parents{ &solutionA, &solutionB };
    eastl::array<
    eastl::function<void(Solution&, const Solution&, const int)>,
    3> solutionFuncs = {
        [](Solution& solution,
           const Solution& source,
           const int buildingIndex) -> void {
          solution.setBuildingXPos(buildingIndex,
                                   source.getBuildingXPos(buildingIndex));
        },
        [](Solution& solution,
           const Solution& source,
           const int buildingIndex) -> void {
          solution.setBuildingYPos(buildingIndex,
                                   source.getBuildingYPos(buildingIndex));
        },
        [](Solution& solution,
           const Solution& source,
           const int buildingIndex) -> void {
          solution.setBuildingRotation(
              buildingIndex,
              source.getBuildingRotation(buildingIndex));
        }
    };

    eastl::array<Solution, 2> children{ solutionA, solutionB };
    for (int childIdx = 0; childIdx < children.size(); childIdx++) {
      do {
        for (int i = 0; i < numBuildings; i++) {
          for (auto& f : solutionFuncs) {
            int parentIdx = cx::generateRandomInt(parentDistrib);
            f(children[childIdx], *parents[parentIdx], i);
          }
        }
      } while (!this->isSolutionFeasible(children[childIdx],
                                         boundingArea,
                                         inputBuildings));
    }

    return children;
  }

  void GA::mutateSolution(Solution& solution,
                          const cx::NPolygon& boundingArea,
                          const eastl::vector<InputBuilding>& inputBuildings)
  {
    eastl::array<eastl::function<void(Solution&,
    const cx::NPolygon&,
    const eastl::vector<InputBuilding>&)>,
    3> mutationFunctions = {
        [this](Solution& solution,
               const cx::NPolygon& boundingArea,
               const eastl::vector<InputBuilding>& inputBuildings)
        {
          this->applyBuddyBuddyMutation(solution, boundingArea, inputBuildings);
        },
        [this](Solution& solution,
               const cx::NPolygon& boundingArea,
               const eastl::vector<InputBuilding>& inputBuildings)
        {
          this->applyShakingMutation(solution, boundingArea, inputBuildings);
        },
        [this](Solution& solution,
               const cx::NPolygon& boundingArea,
               const eastl::vector<InputBuilding>& inputBuildings)
        {
          this->applyJiggleMutation(solution, boundingArea, inputBuildings);
        }
    };

    std::uniform_int_distribution<const int> numMutationsDistrib{
        0, static_cast<int>(mutationFunctions.size() - 1)
    };
    const int mutationFuncIndex = cx::generateRandomInt(
        numMutationsDistrib);
    mutationFunctions[mutationFuncIndex](solution,
                                         boundingArea,
                                         inputBuildings);
  }

  void GA::applyBuddyBuddyMutation(
      Solution& solution,
      const cx::NPolygon& boundingArea,
      const eastl::vector<InputBuilding>& inputBuildings)
  {
    std::uniform_int_distribution<int> buildingDistrib{
        0, static_cast<int>(inputBuildings.size() - 1)
    };
    std::uniform_int_distribution<int> buddySideDistrib{ 0, 3 };
    std::uniform_int_distribution<int> relOrientationDistrib{ 0, 1 };
    std::uniform_real_distribution<float> normalizedDistrib{ 0, 1 };

    Solution tempSolution;
    do {
      tempSolution = solution;

      // Let's just do the Buddy-Buddy Mutation for now.
      int staticBuddy = 0;
      int dynamicBuddy = 0; // The buddy to be moved.
      do {
        staticBuddy = cx::generateRandomInt(buildingDistrib);
        dynamicBuddy = cx::generateRandomInt(buildingDistrib);
      } while (staticBuddy == dynamicBuddy);

      auto staticBuddyRect = cx::Rectangle{
          solution.getBuildingXPos(staticBuddy),
          solution.getBuildingYPos(staticBuddy),
          inputBuildings[staticBuddy].width,
          inputBuildings[staticBuddy].length,
          solution.getBuildingRotation(staticBuddy)
      };
      auto buddyPoly = cx::convertRectangleToPolygon(staticBuddyRect);

      const int buddySide = cx::generateRandomInt(
          buddySideDistrib);

      cx::Line contactLine;
      switch (buddySide) {
        case 0:
          contactLine = cx::Line{
              { buddyPoly.vertices[0].x, buddyPoly.vertices[0].y },
              { buddyPoly.vertices[1].x, buddyPoly.vertices[1].y }
          };
          break;
        case 1:
          contactLine = cx::Line{
              { buddyPoly.vertices[1].x, buddyPoly.vertices[1].y },
              { buddyPoly.vertices[2].x, buddyPoly.vertices[2].y }
          };
          break;
        case 2:
          contactLine = cx::Line{
              { buddyPoly.vertices[2].x, buddyPoly.vertices[2].y },
              { buddyPoly.vertices[3].x, buddyPoly.vertices[3].y }
          };
          break;
        case 3:
          contactLine = cx::Line{
              { buddyPoly.vertices[3].x, buddyPoly.vertices[3].y },
              { buddyPoly.vertices[0].x, buddyPoly.vertices[0].y }
          };
          break;
        default:
          break;
      }

      auto contactLineVec = cx::lineToVec(contactLine);
      const int orientation = cx::generateRandomInt(
          relOrientationDistrib);
      float distContactToBuddyCenter = 0.f;

      // Length to add to both ends of the contact line vector to allow the
      // edges in the dynamic buddy perpendicular to contact line to be in line
      // with those edges parallel to it in the static buddy.
      float extLength = 0.f;

      float contactLineAngle = cx::vec2Angle(contactLineVec);
      float dynamicBuddyAngle;
      if (orientation == 0) {
        // The dynamic buddy will be oriented parallel to the contact line, if
        // width > length. Perpendicular, otherwise.
        distContactToBuddyCenter = inputBuildings[dynamicBuddy].width / 2.f;
        extLength = inputBuildings[dynamicBuddy].length / 2.f;
        dynamicBuddyAngle = contactLineAngle;
      } else if (orientation == 1) {
        // The dynamic buddy will be oriented perpendicular to the contact line,
        // if length > width. Parallel, otherwise.
        distContactToBuddyCenter = inputBuildings[dynamicBuddy].length / 2.f;
        extLength = inputBuildings[dynamicBuddy].width / 2.f;
        dynamicBuddyAngle = contactLineAngle + 45.f;
      }

      // Adjust the distance of the dynamic buddy centroid to the contact line
      // by a small amount to prevent intersection of buildings.
      distContactToBuddyCenter += 0.0001f;

      auto buddyMidptRelContactLine = cx::rotateVec2(
          cx::Vec2{0.f, extLength * 2 }, contactLineAngle)
                                      + contactLineVec;
      auto buddyMidptRelContactLineStart = cx::rotateVec2(
          cx::Vec2{ 0.f, -extLength }, contactLineAngle)
                                           + contactLine.start;

      const float lineWidthModifier = cx::generateRandomReal(
          normalizedDistrib);

      cx::Point dynamicBuddyPos{
          ((buddyMidptRelContactLine * lineWidthModifier)
           + cx::vec2Perp(
              cx::rotateVec2(
                  cx::Vec2{ 0.f, distContactToBuddyCenter },
                  contactLineAngle)))
          + buddyMidptRelContactLineStart
      };

      tempSolution.setBuildingXPos(dynamicBuddy, dynamicBuddyPos.x);
      tempSolution.setBuildingYPos(dynamicBuddy, dynamicBuddyPos.y);
      tempSolution.setBuildingRotation(dynamicBuddy, dynamicBuddyAngle);
    } while (!this->isSolutionFeasible(tempSolution,
                                       boundingArea,
                                       inputBuildings));
    solution = tempSolution;
  }

  void GA::applyShakingMutation(
      Solution& solution,
      const cx::NPolygon& boundingArea,
      const eastl::vector<InputBuilding>& inputBuildings)
  {
    std::uniform_int_distribution<int> geneDistribution{
        0, solution.getNumBuildings() - 1
    };

    int targetGeneIndex = cx::generateRandomInt(geneDistribution);

    float minX = std::min_element(
        boundingArea.vertices.begin(),
        boundingArea.vertices.end(),
        [](cx::Point ptA, cx::Point ptB) -> bool {
          return ptA.x < ptB.x;
        }
    )->x;
    float maxX = std::max_element(
        boundingArea.vertices.begin(),
        boundingArea.vertices.end(),
        [](cx::Point ptA, cx::Point ptB) -> bool {
          return ptA.x < ptB.x;
        }
    )->x;
    float minY = std::min_element(
        boundingArea.vertices.begin(),
        boundingArea.vertices.end(),
        [](cx::Point ptA, cx::Point ptB) -> bool {
          return ptA.y < ptB.y;
        }
    )->y;
    float maxY = std::max_element(
        boundingArea.vertices.begin(),
        boundingArea.vertices.end(),
        [](cx::Point ptA, cx::Point ptB) -> bool {
          return ptA.y < ptB.y;
        }
    )->y;

    std::uniform_real_distribution<float> xPosDistribution{ minX, maxX };
    std::uniform_real_distribution<float> yPosDistribution{ minY, maxY };
    std::uniform_real_distribution<float> rotationDistribution{ 0.f, 360.f };

    Solution tempSolution = solution;
    do {
      float newXPos = cx::generateRandomReal(xPosDistribution);
      float newYPos = cx::generateRandomReal(yPosDistribution);
      float newRotation = cx::generateRandomReal(rotationDistribution);

      tempSolution.setBuildingXPos(targetGeneIndex, newXPos);
      tempSolution.setBuildingYPos(targetGeneIndex, newYPos);
      tempSolution.setBuildingRotation(targetGeneIndex, newRotation);
    } while (!this->isSolutionFeasible(tempSolution,
                                       boundingArea,
                                       inputBuildings));

    solution = tempSolution;
  }

  void GA::applyJiggleMutation(
      Solution& solution,
      const cx::NPolygon& boundingArea,
      const eastl::vector<InputBuilding>& inputBuildings)
  {
    Solution tempSolution;
    do {
      tempSolution = solution;

      constexpr int numMovements = 8;
      constexpr float maxShiftAmount = 1.f;
      constexpr float maxRotShiftAmount = 5.f;
      std::uniform_real_distribution<float> shiftDistrib{ 0, maxShiftAmount };
      std::uniform_int_distribution<int> buildingIndexDistrib{
          0, static_cast<int>(inputBuildings.size() - 1)
      };
      std::uniform_real_distribution<float> rotShiftDistrib{
          -maxRotShiftAmount,
          maxRotShiftAmount
      };
      static const
      eastl::array<eastl::function<Solution(Solution, int)>,
          numMovements> jiggleFunctions = {
          [&shiftDistrib](Solution solution, int buildingIndex) -> Solution
          {
            float shiftAmount = cx::generateRandomReal(shiftDistrib);
            solution.setBuildingXPos(buildingIndex,
                                     solution.getBuildingXPos(buildingIndex)
                                     + shiftAmount);
            return solution;
          },
          [&shiftDistrib](Solution solution, int buildingIndex) -> Solution
          {
            float shiftAmount = cx::generateRandomReal(shiftDistrib);
            solution.setBuildingXPos(buildingIndex,
                                     solution.getBuildingXPos(buildingIndex)
                                     - shiftAmount);
            return solution;
          },
          [&shiftDistrib](Solution solution, int buildingIndex) -> Solution
          {
            float shiftAmount = cx::generateRandomReal(shiftDistrib);

            // NOTE: The origin is on the top left corner.
            solution.setBuildingYPos(buildingIndex,
                                     solution.getBuildingXPos(buildingIndex)
                                     - shiftAmount);
            return solution;
          },
          [&shiftDistrib](Solution solution, int buildingIndex) -> Solution
          {
            float shiftAmount = cx::generateRandomReal(shiftDistrib);

            // NOTE: The origin is on the top left corner.
            solution.setBuildingYPos(buildingIndex,
                                     solution.getBuildingXPos(buildingIndex)
                                     + shiftAmount);
            return solution;
          },
          [&shiftDistrib](Solution solution, int buildingIndex) -> Solution
          {
            float shiftAmountA = cx::generateRandomReal(shiftDistrib);
            float shiftAmountB = cx::generateRandomReal(shiftDistrib);
            solution.setBuildingXPos(buildingIndex,
                                     solution.getBuildingXPos(buildingIndex)
                                     + shiftAmountA);

            // NOTE: The origin is on the top left corner.
            solution.setBuildingYPos(buildingIndex,
                                     solution.getBuildingYPos(buildingIndex)
                                     - shiftAmountB);
            return solution;
          },
          [&shiftDistrib](Solution solution, int buildingIndex) -> Solution
          {
            float shiftAmountA = cx::generateRandomReal(shiftDistrib);
            float shiftAmountB = cx::generateRandomReal(shiftDistrib);
            solution.setBuildingXPos(buildingIndex,
                                     solution.getBuildingXPos(buildingIndex)
                                     + shiftAmountA);

            // NOTE: The origin is on the top left corner.
            solution.setBuildingYPos(buildingIndex,
                                     solution.getBuildingYPos(buildingIndex)
                                     + shiftAmountB);
            return solution;
          },
          [&shiftDistrib](Solution solution, int buildingIndex) -> Solution
          {
            float shiftAmountA = cx::generateRandomReal(shiftDistrib);
            float shiftAmountB = cx::generateRandomReal(shiftDistrib);
            solution.setBuildingXPos(buildingIndex,
                                     solution.getBuildingXPos(buildingIndex)
                                     - shiftAmountA);

            // NOTE: The origin is on the top left corner.
            solution.setBuildingYPos(buildingIndex,
                                     solution.getBuildingYPos(buildingIndex)
                                     - shiftAmountB);
            return solution;
          },
          [&shiftDistrib](Solution solution, int buildingIndex) -> Solution
          {
            float shiftAmountA = cx::generateRandomReal(shiftDistrib);
            float shiftAmountB = cx::generateRandomReal(shiftDistrib);
            solution.setBuildingXPos(buildingIndex,
                                     solution.getBuildingXPos(buildingIndex)
                                     + shiftAmountA);

            // NOTE: The origin is on the top left corner.
            solution.setBuildingYPos(buildingIndex,
                                     solution.getBuildingYPos(buildingIndex)
                                     + shiftAmountB);
            return solution;
          }
      };
      std::uniform_int_distribution<int> jiggleFuncDistrib{
          0, static_cast<int>(jiggleFunctions.size() - 1)
      };

      const int targetBuildingIndex = cx::generateRandomInt(
          buildingIndexDistrib);
      const int jiggleFuncIndex = cx::generateRandomInt(
          jiggleFuncDistrib);

      tempSolution = jiggleFunctions[jiggleFuncIndex](tempSolution,
                                                      targetBuildingIndex);

      const float rotDelta = cx::generateRandomReal(rotShiftDistrib);
      const float newRot = tempSolution.getBuildingRotation(targetBuildingIndex)
                           + rotDelta;
      tempSolution.setBuildingRotation(targetBuildingIndex, newRot);
    } while (!this->isSolutionFeasible(tempSolution,
                                       boundingArea,
                                       inputBuildings));

    solution = tempSolution;
  }

  bool GA::isSolutionFeasible(
      const Solution& solution,
      const cx::NPolygon& boundingArea,
      const eastl::vector<InputBuilding>& inputBuildings)
  {
    return this->doesSolutionHaveNoBuildingsOverlapping(solution,
                                                        inputBuildings)
           && this->areSolutionBuildingsWithinBounds(solution,
                                                     boundingArea,
                                                     inputBuildings);
  }

  bool GA::doesSolutionHaveNoBuildingsOverlapping(
      const Solution& solution,
      const eastl::vector<InputBuilding>& inputBuildings)
  {
    for (int i = 0; i < solution.getNumBuildings(); i++) {
      cx::Rectangle building0 = cx::Rectangle{
          solution.getBuildingXPos(i),
          solution.getBuildingYPos(i),
          inputBuildings[i].width,
          inputBuildings[i].length,
          solution.getBuildingRotation(i)
      };

      for (int j = i + 1; j < solution.getNumBuildings(); j++) {
        cx::Rectangle building1 = cx::Rectangle{
            solution.getBuildingXPos(j),
            solution.getBuildingYPos(j),
            inputBuildings[j].width,
            inputBuildings[j].length,
            solution.getBuildingRotation(j)
        };
        if (cx::areTwoRectsIntersecting(building0, building1)) {
          return false;
        }
      }
    }

    return true;
  }

  bool GA::areSolutionBuildingsWithinBounds(
      const Solution& solution,
      const cx::NPolygon& boundingArea,
      const eastl::vector<InputBuilding>& inputBuildings)
  {
    for (int i = 0; i < solution.getNumBuildings(); i++) {
      cx::Rectangle buildingRect = cx::Rectangle{
          solution.getBuildingXPos(i),
          solution.getBuildingYPos(i),
          inputBuildings[i].width,
          inputBuildings[i].length,
          solution.getBuildingRotation(i)
      };

      if (!cx::isRectWithinNPolygon(buildingRect, boundingArea)) {
        return false;
      }
    }

    return true;
  }
}
