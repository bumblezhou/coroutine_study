#include <coroutine>
#include <iostream>
 
struct promise;
 
struct coroutine : std::coroutine_handle<promise>
{
    using promise_type = ::promise;
};
 
struct promise
{
    coroutine get_return_object() { return {coroutine::from_promise(*this)}; }
    std::suspend_always initial_suspend() noexcept { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void return_void() {}
    void unhandled_exception() {}
};
 
void good()
{
    coroutine h = [](int i) -> coroutine // make i a coroutine parameter
    {
        std::cout << "i: " << i << "\n";
        std::cout << "co_return" << "\n";
        co_return;
    }(0);
    // lambda destroyed
    std::cout << "h.resume()" << "\n";
    h.resume(); // no problem, i has been copied to the coroutine
                // frame as a by-value parameter
    std::cout << "h.destroy()" << "\n";
    h.destroy();
}
 
int main()
{
  std::cout << "main start ..." << "\n";
  good();
  std::cout << "main end ..." << "\n";
  return 0;
}