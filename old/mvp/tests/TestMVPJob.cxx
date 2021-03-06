#include <gtest/gtest.h>
#include <test/Helpers.h>

#include <mvp/MVPJob.h>
#include <vw/Plate/PlateGeoReference.h>

using namespace std;
using namespace vw;
using namespace vw::test;
using namespace vw::cartography;
using namespace vw::platefile;
using namespace vw::camera;
using namespace mvp;

MVPJobRequest create_job_request(bool use_octave = false) {
  // Set up workspace
  const int tile_size = 64;
  const Datum datum("D_MOON");

  // Same tile as TestMVPAlgorithm
  const int col = 5374;
  const int row = 3875;
  const int level = 13;

  PlateGeoReference plate_georef(datum, "equi", tile_size, GeoReference::PixelAsPoint);
  MVPUserSettings user_settings;
  user_settings.set_alt_min(0);
  user_settings.set_alt_max(0);
  user_settings.set_alt_search_range(0);
  user_settings.set_seed_window_smooth_size(0);
  user_settings.set_window_size(0);
  user_settings.set_window_smooth_size(0);

  MVPJobRequest job_request;
  job_request.set_col(col);
  job_request.set_row(row);
  job_request.set_level(level);
  job_request.set_tile_size(tile_size);
  job_request.set_result_platefile("result");
  job_request.set_internal_result_platefile("internal");
  *job_request.mutable_georef() = plate_georef.tile_georef(col, row, level).build_desc();
  *job_request.mutable_user_settings() = user_settings;

  for (int i = 0; i <= 3; i++) {
    stringstream ss;
    ss << "synth." << i;
    OrbitalImageFileDescriptor image;
    image.set_image_path(SrcName(ss.str() + ".tif"));
    image.set_camera_path(SrcName(ss.str() + ".pinhole"));
    *job_request.add_orbital_images() = image;
  }

  return job_request;
}

TEST(MVPJob, save_job_file) {
  // TODO: Finish test here
  MVPJob job(create_job_request());
  string job_file = job.save_job_file();
}

TEST(MVPJob, load_job_file) {
  // TODO: write me!
}
