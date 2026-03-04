#include "ap.h"
#include "dataanalysis.h"
#include <cstdint>
namespace forest {
extern alglib::real_2d_array xy;
extern bool merge_loaded;
extern alglib::decisionforest forest_model;

void tree(int8_t *values);
// int load_forest_task(int argc, char **argv);
}; // namespace forest
