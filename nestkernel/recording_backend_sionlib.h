/*
 *  recording_backend_sionlib.h
 *
 *  This file is part of NEST.
 *
 *  Copyright (C) 2004 The NEST Initiative
 *
 *  NEST is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  NEST is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with NEST.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef RECORDING_BACKEND_SIONLIB_H
#define RECORDING_BACKEND_SIONLIB_H

// C includes:
#include <mpi.h>
#include <sion.h>

#include "recording_backend.h"

namespace nest
{
class RecordingBackendSIONlib : public RecordingBackend
{
public:
  RecordingBackendSIONlib()
    : files_()
  {
  }

  RecordingBackendSIONlib( std::string file_ext,
    long buffer_size,
    long sion_chunksize,
    bool sion_collective )
    : files_()
  {
    P_.file_ext_ = file_ext;
    P_.buffer_size_ = buffer_size;
    P_.sion_chunksize_ = sion_chunksize;
    P_.sion_collective_ = sion_collective;
  }

  ~RecordingBackendSIONlib() throw()
  {
  }

  void enroll( const RecordingDevice& device );
  void enroll( const RecordingDevice& device,
    const std::vector< Name >& value_names );

  void finalize();
  void synchronize();

  void write( const RecordingDevice& device, const Event& event );
  void write( const RecordingDevice& device,
    const Event& event,
    const std::vector< double >& );

  void set_status( const DictionaryDatum& );
  void get_status( DictionaryDatum& ) const;

  void initialize();

private:
  void close_files_();
  const std::string build_filename_() const;

  class SIONBuffer
  {
  private:
    // TODO: add underscores
    char* buffer;
    size_t ptr;
    size_t max_size;

  public:
    SIONBuffer();
    SIONBuffer( size_t size );
    ~SIONBuffer();

    void reserve( size_t size );
    void ensure_space( size_t size );
    void write( const char* v, size_t n );

    size_t
    get_capacity()
    {
      return max_size;
    };
    size_t
    get_size()
    {
      return ptr;
    };
    size_t
    get_free()
    {
      return max_size - ptr;
    };
    void
    clear()
    {
      ptr = 0;
    };
    char*
    read()
    {
      return buffer;
    };
    template < typename T >
    SIONBuffer& operator<<( const T data );
  };

  struct DeviceInfo
  {
    DeviceInfo()
      : n_rec( 0 )
    {
    }

    index gid;
    unsigned int type;
    std::string name;
    std::string label;
    unsigned long int n_rec;
    std::vector< std::string > value_names;
  };

  struct DeviceEntry
  {
    DeviceEntry( RecordingDevice& device )
      : device( device )
      , info()
    {
    }

    RecordingDevice& device;
    DeviceInfo info;
  };

  struct FileEntry
  {
    int sid;
    SIONBuffer buffer;
  };

  typedef std::map< thread, std::map< index, DeviceEntry > > device_map;
  device_map devices_;

  typedef std::map< thread, FileEntry > file_map;
  file_map files_;

  std::string filename_;

  double t_start_; // simulation start time for storing

  struct Parameters_
  {
    std::string file_ext_; //!< the file name extension to use, without .
    bool sion_collective_; //!< use SIONlib's collective mode.
    long sion_chunksize_;  //!< the size of SIONlib's buffer.
    long buffer_size_;     //!< the size of the internal buffer.

    Parameters_();

    void get( const RecordingBackendSIONlib&, DictionaryDatum& ) const;
    void set( const RecordingBackendSIONlib&, const DictionaryDatum& );
  };

  Parameters_ P_;
};

inline void
RecordingBackendSIONlib::get_status( DictionaryDatum& d ) const
{
  P_.get( *this, d );

  ( *d )[ names::filename ] = filename_;
}

} // namespace

#endif // RECORDING_BACKEND_SIONLIB_H
