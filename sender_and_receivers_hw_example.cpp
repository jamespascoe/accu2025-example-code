#include <cstdint>
#include <cstdlib>
#include <stdexec/execution.hpp>
#include <exec/task.hpp>
#include <exec/into_tuple.hpp>

/// Old-style async C API with a callback (like Win32's ReadFileEx)
using handle_t = std::uintptr_t;
constexpr handle_t invalid_handle = -1u;

constexpr unsigned OK = 0;
constexpr unsigned READ = 1;
constexpr unsigned WRITE = 2;

struct overlapped {};
using overlapped_callback = void(int status, int bytes, overlapped* user);
int get_overlapped_result(overlapped*);

handle_t open_file(char const* name, int mode) {
  std::printf("Opening file: %s in mode %d\n", name, mode);
  return 0;
};

int read_file(handle_t, char* buffer, int bytes, overlapped*, overlapped_callback* pfn) {
  std::printf("Reading %d bytes from file\n", bytes);
  return OK;
};


using namespace stdexec::tags;

struct immovable {
  immovable() = default;
  immovable(immovable&&) = delete;
};

/// A sender that wraps the C-style API that can then be passed to
/// generic algorithms.
template <class Receiver>
struct operation : overlapped, immovable {
  handle_t file;
  char* buffer;
  int size;
  Receiver rcvr;

  static void callback(int status, int bytes, overlapped* user) {
    operation* op = static_cast<operation*>(user);
    if (status != OK)
      stdexec::set_error(std::move(op->rcvr), status);
    else
      stdexec::set_value(std::move(op->rcvr), bytes, op->buffer);
  }

  STDEXEC_MEMFN_DECL(void start)(this operation& self) noexcept {
    int status = read_file(self.file, self.buffer, self.size, &self, &callback);
    if (status != OK)
      stdexec::set_error(std::move(self.rcvr), status);
  }
};

struct read_sender {
  using sender_concept = stdexec::sender_t;

  using completion_signatures =
    stdexec::completion_signatures<
      stdexec::set_value_t(int, char*),
      stdexec::set_error_t(int)>;

  STDEXEC_MEMFN_DECL(auto connect)(this read_sender self, stdexec::receiver auto rcvr) {
    return operation{{}, {}, self.file, self.buffer, self.size, std::move(rcvr)};
  };

  handle_t file;
  char* buffer;
  int size;
};

read_sender async_read(handle_t file, char* buffer, int size) {
  return {file, buffer, size};
}

exec::task<void> co_read_some(handle_t file, char* buffer, int size) {
  auto [bytes, buff] = co_await async_read(file, buffer, size);
}

int main() {

  std::vector<char> data(256);

  auto file = open_file("hello.txt", READ);
  auto task = async_read(0, data.data(), data.size());

  auto [size, buffer] = stdexec::sync_wait(task).value();
}
