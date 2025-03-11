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
    std::printf("Finished timer for %d seconds\n", n_secs);
    if (error != boost::system::errc::success)
      stdexec::set_error(std::move(rcvr), error);
    else
      stdexec::set_value(std::move(rcvr), 0);
  }

  STDEXEC_MEMFN_DECL(void start)(this timer_op& self) noexcept {
    self.timer.async_wait(boost::bind(callback, boost::asio::placeholders::error, self.n_secs, self.rcvr));
    std::printf("Running\n");
    self.io.run();
  }
};

struct timer_sender {
  using sender_concept = stdexec::sender_t;

  using completion_signatures =
    stdexec::completion_signatures<
      stdexec::set_value_t(int),
      stdexec::set_error_t(int)>;

  STDEXEC_MEMFN_DECL(auto connect)(this timer_sender self, stdexec::receiver auto rcvr) {
    asio::steady_timer timer(self.io, asio::chrono::seconds(self.n_secs));

    std::printf("setup done for %d seconds\n", self.n_secs);
    return timer_op{self.io, self.n_secs, std::move(timer), std::move(rcvr)};
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
  auto ret = stdexec::sync_wait(task_timer).value();
}
