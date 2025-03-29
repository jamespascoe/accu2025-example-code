#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <stdexec/execution.hpp>
#include <exec/task.hpp>

using namespace stdexec::tags;

namespace asio = boost::asio;

template <class Receiver>
struct timer_op {
  asio::io_context& io;
  int n_secs;
  asio::steady_timer timer;
  Receiver rcvr;

  static void callback(const boost::system::error_code& error, int n_secs, Receiver& rcvr)
  {
    std::printf("Finished %d second timer\n", n_secs);
    if (error != boost::system::errc::success)
      stdexec::set_error(std::move(rcvr), error);
    else
      stdexec::set_value(std::move(rcvr), 0);
  }

  void start() noexcept {
    timer.async_wait(boost::bind(callback, boost::asio::placeholders::error, n_secs, rcvr));
    std::printf("Running\n");
    io.run();
  }
};

struct timer_sender {
  using sender_concept = stdexec::sender_t;

  using completion_signatures =
    stdexec::completion_signatures<
      stdexec::set_value_t(int),
      stdexec::set_error_t(int)>;

  auto connect(stdexec::receiver auto rcvr) {
    asio::steady_timer timer(io, asio::chrono::seconds(n_secs));

    std::printf("Setup done for %d second timer\n", n_secs);
    return timer_op{io, n_secs, std::move(timer), std::move(rcvr)};
  };

  asio::io_context& io;
  int n_secs;
};

timer_sender async_timer(asio::io_context& io, int n_secs) {
  return {io, n_secs};
}

int main() {
  asio::io_context io;

  auto task_timer = async_timer(io, 5);
  auto print_ret_val = stdexec::then(task_timer, [](int n)
        {
          std::printf("Return value: %d\n", n);
        });
  auto ret = stdexec::sync_wait(print_ret_val).value();
}
