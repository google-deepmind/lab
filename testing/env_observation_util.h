#ifndef DML_TESTING_ENV_OBSERVATION_UTIL_H_
#define DML_TESTING_ENV_OBSERVATION_UTIL_H_

#include <string>

#include "testing/env_observation.h"

namespace deepmind {
namespace lab {

// Compares two observations, returning a number representing their similarity.
// Observations shall be bytes of shape [h, w, c] (with c being the number of
// colour components).
// Both observations are resampled to shape [grid_size, grid_size, c] and the
// L1 norm of the component-wise difference between the observations is
// returned.
// If there are three colour components and you want each component to be on
// average less than 1% different, then your comparison number would be:
//   grid_size * grid_size * 3 * (0.01 * 255)
double CompareInterlacedObservations(const EnvObservation<unsigned char>& lhs,
                                     const EnvObservation<unsigned char>& rhs,
                                     int grid_size);

// Writes an image to the path specified with the contents of the observation.
// Observations shall be bytes of shape [h, w, 3].
void SaveInterlacedRGBObservationToJpg(
    const EnvObservation<unsigned char>& obs, const std::string& path);

}  // namespace lab
}  // namespace deepmind

#endif  // DML_TESTING_ENV_OBSERVATION_UTIL_H_
