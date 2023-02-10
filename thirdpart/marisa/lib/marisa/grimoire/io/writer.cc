#include <stdio.h>

#ifdef _WIN32
 #include <io.h>
#else  // _WIN32
 #include <unistd.h>
#endif  // _WIN32

#include <limits>

#include "lib/marisa/grimoire/io/writer.h"

namespace marisa {
namespace grimoire {
namespace io {

#ifdef MARISA_SUPPORT_CPP_STREAM
Writer::Writer()
    : file_(NULL), fd_(-1), stream_(NULL), needs_fclose_(false), save_buffer_(false) {}
#else
Writer::Writer()
    : file_(NULL), fd_(-1), needs_fclose_(false), save_buffer_(false) {}
#endif

Writer::~Writer() {
  if (needs_fclose_) {
    ::fclose(file_);
  }
}

void Writer::open(const char *filename) {
  MARISA_THROW_IF(filename == NULL, MARISA_NULL_ERROR);

  Writer temp;
  temp.open_(filename);
  swap(temp);
}

void Writer::open(std::FILE *file) {
  MARISA_THROW_IF(file == NULL, MARISA_NULL_ERROR);

  Writer temp;
  temp.open_(file);
  swap(temp);
}

void Writer::open(int fd) {
  MARISA_THROW_IF(fd == -1, MARISA_CODE_ERROR);

  Writer temp;
  temp.open_(fd);
  swap(temp);
}

#ifdef MARISA_SUPPORT_CPP_STREAM
void Writer::open(std::ostream &stream) {
  Writer temp;
  temp.open_(stream);
  swap(temp);
}
#endif

void Writer::open() {
  Writer temp;
  temp.open_();
  swap(temp);	
}

void Writer::clear() {
  Writer().swap(*this);
}

void Writer::swap(Writer &rhs) {
  marisa::swap(file_, rhs.file_);
  marisa::swap(fd_, rhs.fd_);
#ifdef MARISA_SUPPORT_CPP_STREAM
  marisa::swap(stream_, rhs.stream_);
#endif
  marisa::swap(needs_fclose_, rhs.needs_fclose_);
  marisa::swap(buffer_, rhs.buffer_);
  marisa::swap(save_buffer_, rhs.save_buffer_);
}

void Writer::seek(std::size_t size) {
  MARISA_THROW_IF(!is_open(), MARISA_STATE_ERROR);
  if (size == 0) {
    return;
  } else if (size <= 16) {
    const char buf[16] = {};
    write_data(buf, size);
  } else {
    const char buf[1024] = {};
    do {
      const std::size_t count = (size < sizeof(buf)) ? size : sizeof(buf);
      write_data(buf, count);
      size -= count;
    } while (size != 0);
  }
}

bool Writer::is_open() const {
#ifdef MARISA_SUPPORT_CPP_STREAM
  return (file_ != NULL) || (fd_ != -1) || (stream_ != NULL) || save_buffer_;
#else
  return (file_ != NULL) || (fd_ != -1) || save_buffer_;
#endif
}

void Writer::open_(const char *filename) {
  std::FILE *file = NULL;
#ifdef _MSC_VER
  MARISA_THROW_IF(::fopen_s(&file, filename, "wb") != 0, MARISA_IO_ERROR);
#else  // _MSC_VER
  file = ::fopen(filename, "wb");
  MARISA_THROW_IF(file == NULL, MARISA_IO_ERROR);
#endif  // _MSC_VER
  file_ = file;
  needs_fclose_ = true;
}

void Writer::open_(std::FILE *file) {
  file_ = file;
}

void Writer::open_(int fd) {
  fd_ = fd;
}

#ifdef MARISA_SUPPORT_CPP_STREAM
void Writer::open_(std::ostream &stream) {
  stream_ = &stream;
}
#endif

void Writer::open_() {
	save_buffer_ = true;
}	  


void Writer::write_data(const void *data, std::size_t size) {
  MARISA_THROW_IF(!is_open(), MARISA_STATE_ERROR);
  if (size == 0) {
    return;
  } else if (fd_ != -1) {
    while (size != 0) {
#ifdef _WIN32
      static const std::size_t CHUNK_SIZE =
          std::numeric_limits<int>::max();
      const unsigned int count = (size < CHUNK_SIZE) ? size : CHUNK_SIZE;
      const int size_written = ::_write(fd_, data, count);
#else  // _WIN32
      static const std::size_t CHUNK_SIZE =
          std::numeric_limits< ::ssize_t>::max();
      const ::size_t count = (size < CHUNK_SIZE) ? size : CHUNK_SIZE;
      const ::ssize_t size_written = ::write(fd_, data, count);
#endif  // _WIN32
      MARISA_THROW_IF(size_written <= 0, MARISA_IO_ERROR);
      data = static_cast<const char *>(data) + size_written;
      size -= size_written;
    }
  } else if (file_ != NULL) {
    MARISA_THROW_IF(::fwrite(data, 1, size, file_) != size, MARISA_IO_ERROR);
    MARISA_THROW_IF(::fflush(file_) != 0, MARISA_IO_ERROR);
  }
#ifdef MARISA_SUPPORT_CPP_STREAM
  else if (stream_ != NULL) {
    try {
      MARISA_THROW_IF(!stream_->write(static_cast<const char *>(data), size),
          MARISA_IO_ERROR);
    } catch (const std::ios_base::failure &) {
      MARISA_THROW(MARISA_IO_ERROR, "std::ios_base::failure");
    }
  }
#endif
  else if(save_buffer_) {
	buffer_.append((const char*)data, size);
  }
}

}  // namespace io
}  // namespace grimoire
}  // namespace marisa
