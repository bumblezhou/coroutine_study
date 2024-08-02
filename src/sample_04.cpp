#include <coroutine>
#include <iostream>
#include <stdexcept>
#include <thread>
 
auto switch_to_new_thread(std::jthread& out)
{
  struct awaitable
  {
    std::jthread* p_out;
    bool await_ready()
    {
      std::cout << "await_ready() return false; \n";
      return false;
    }
    void await_suspend(std::coroutine_handle<> h)
    {
      std::jthread& out = *p_out;
      if (out.joinable())
      {
        throw std::runtime_error("Output jthread parameter not empty");
      }
      out = std::jthread([h] {
        std::cout << "jthread is running, thread ID: " << std::this_thread::get_id() << "\n";
        h.resume();
      });
      // Potential undefined behavior: accessing potentially destroyed *this
      // std::cout << "New thread ID: " << p_out->get_id() << '\n';
      std::cout << "await_suspend() New thread ID: " << out.get_id() << '\n'; // this is OK
    }
    void await_resume()
    {
      std::cout << "await_resume() New thread ID: " << p_out->get_id() << '\n';
    }
  };
  return awaitable{&out};
}
 
struct task
{
  struct promise_type
  {
    task get_return_object()
    {
      std::cout << "provise_type.get_return_object()\n";
      return {}; 
    }
    std::suspend_never initial_suspend()
    {
      std::cout << "provise_type.initial_suspend()\n";
      return {};
    }
    std::suspend_never final_suspend() noexcept
    {
      std::cout << "promise_type.final_suspend()\n";
      return {};
    }
    void return_void()
    {
      std::cout << "promise_type.return_void()\n";
    }
    void unhandled_exception()
    {
      std::cout << "promise_type.unhandled_exception()\n";
    }
  };
};
 
task resuming_on_new_thread(std::jthread& out)
{
    std::cout << "Coroutine started on thread: " << std::this_thread::get_id() << '\n';
    co_await switch_to_new_thread(out);
    // awaiter destroyed here
    std::cout << "Coroutine resumed on thread: " << std::this_thread::get_id() << '\n';
}
 
int main()
{
  std::cout << "main start on thread: " << std::this_thread::get_id() << '\n';

  std::jthread out;
  resuming_on_new_thread(out);

  std::cout << "main sleep for 100 milliseconds\n";
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  std::cout << "main end on thread: " << std::this_thread::get_id() << '\n';
  return 0;
}