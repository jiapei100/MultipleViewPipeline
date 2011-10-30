/// \file MVPJobBase.h
///
/// MVP Job Base class
///
/// TODO: Write something here
///

#ifndef __MVP_MVPJOBBASE_H__
#define __MVP_MVPJOBBASE_H__

#include <vw/Plate/PlateGeoReference.h>
#include <vw/Cartography/GeoReference.h>
#include <vw/Cartography/SimplePointImageManipulation.h>
#include <vw/Image/PixelMask.h>

#include <mvp/MVPJobRequest.pb.h>
#include <mvp/OrbitalImageCrop.h>

#include <boost/foreach.hpp>

namespace mvp {

// TODO: This should be replaced by a function in VW
inline vw::cartography::GeoReference offset_georef(vw::cartography::GeoReference const& georef, int cols, int rows) {
  vw::Matrix3x3 offset;

  offset.set_identity();
  offset(0, 2) = cols;
  offset(1, 2) = rows;

  vw::cartography::GeoReference result(georef);
  // TODO: This is slow...
  result.set_transform(georef.transform() * offset);
  return result; 
}


struct MVPAlgorithmVar {
  vw::float32 post_height;

  vw::Vector3f orientation;
  vw::Vector3f windows;

  MVPAlgorithmVar(vw::float32 h = 0, vw::Vector3f const& o = vw::Vector3f(), vw::Vector3f const& w = vw::Vector3f()) :
    post_height(h), orientation(o), windows(w) {}
  
  // to_octave()
};

struct MVPPixelResult : public MVPAlgorithmVar {
  vw::float32 variance;
  bool converged;
  int num_iterations_to_converge;

  MVPPixelResult(MVPAlgorithmVar const& mav = MVPAlgorithmVar(), vw::float32 v = 0, bool c = true, int n = 0) :
    MVPAlgorithmVar(mav), variance(v), converged(c), num_iterations_to_converge(n) {}

  // constructor from octave vars
};


struct MVPTileResult {
  vw::cartography::GeoReference georef;

  vw::ImageView<vw::PixelMask<vw::PixelGray<vw::float32> > > post_height;
  vw::ImageView<vw::PixelMask<vw::PixelGray<vw::float32> > > variance;

  vw::ImageView<vw::PixelMask<vw::Vector3f> > orientation;
  vw::ImageView<vw::PixelMask<vw::Vector3f> > windows;

  MVPTileResult(vw::cartography::GeoReference g, int tile_size) : georef(g),
    post_height(tile_size, tile_size), variance(tile_size, tile_size), 
    orientation(tile_size, tile_size), windows(tile_size, tile_size) {}

  inline void update(int col, int row, MVPPixelResult const& px_result) {
    post_height(col, row) = px_result.post_height;
    variance(col, row) = px_result.variance;

    orientation(col, row) = px_result.orientation;
    windows(col, row) = px_result.windows;

    if (!px_result.converged) {
      post_height(col, row).invalidate();
      variance(col, row).invalidate();
      orientation(col, row).invalidate();
      windows(col, row).invalidate();
    }
  }
};

template <class ImplT>
struct MVPJobBase {

  static ImplT construct_from_job_request(MVPJobRequest const& job_request) {
    vw::platefile::PlateGeoReference plate_georef(job_request.plate_georef());

    vw::cartography::GeoReference georef(plate_georef.tile_georef(job_request.col(), job_request.row(), job_request.level()));

    vw::BBox2 tile_bbox(plate_georef.tile_lonlat_bbox(job_request.col(), job_request.row(), job_request.level()));

    OrbitalImageCropCollection crops(tile_bbox);
    crops.add_image_collection(job_request.orbital_images());

    return ImplT(georef, plate_georef.tile_size(), crops, job_request.algorithm_settings());
  }

  inline ImplT& impl() {return static_cast<ImplT&>(*this);}
  inline ImplT const& impl() const {return static_cast<ImplT const&>(*this);}

  inline MVPPixelResult process_pixel(MVPAlgorithmVar const& seed, vw::cartography::GeoReference const& georef) const 
    {return impl().process_pixel(seed, georef);}

  inline MVPTileResult process_tile(vw::ProgressCallback const& progress = vw::ProgressCallback::dummy_instance()) {
    // TODO: Seed propigation
    MVPTileResult tile_result(m_georef, m_tile_size);
    MVPAlgorithmVar seed;

    int curr_px_num = 0;
    int num_px_to_process = m_tile_size * m_tile_size;
    progress.report_progress(0);
    for (int col = 0; col < m_tile_size; col++) {
      for (int row = 0; row < m_tile_size; row++) {
        tile_result.update(col, row, process_pixel(seed, offset_georef(m_georef, col, row)));
        progress.report_progress(double(++curr_px_num) / num_px_to_process);
      }
    }
    progress.report_finished();
    return tile_result;
  }

  protected:
    vw::cartography::GeoReference m_georef;
    int m_tile_size;
    OrbitalImageCropCollection m_crops;
    MVPAlgorithmSettings m_settings;
    // TODO: A seed object...

    // This is defined here to prevent the user from accidently
    // constructing an MVPJobBase
    MVPJobBase(vw::cartography::GeoReference const& georef, int tile_size, OrbitalImageCropCollection const& crops, MVPAlgorithmSettings const& settings) :
      m_georef(georef), m_tile_size(tile_size), m_crops(crops), m_settings(settings) {}
    MVPJobBase(MVPJobBase const& job) : m_georef(job.m_georef), m_tile_size(job.m_tile_size), m_crops(job.m_crops), m_settings(job.m_settings) {}

  private:
    // These are defined here to prevent them from being used
    MVPJobBase& operator=(MVPJobBase const&) {return *this;}
};

} // namespace mvp

#endif