#include <coroutine>
#include <cstdint>
#include <exception>
#include <iostream>
 
template<typename T>
struct Generator
{
  // The class name 'Generator' is our choice and it is not required for coroutine
  // magic. Compiler recognizes coroutine by the presence of 'co_yield' keyword.
  // You can use name 'MyGenerator' (or any other name) instead as long as you include
  // nested struct promise_type with 'MyGenerator get_return_object()' method.
  // (Note: It is necessary to adjust the declarations of constructors and destructors
  //  when renaming.)

  struct promise_type;
  using handle_type = std::coroutine_handle<promise_type>;

  struct promise_type // required
  {
    T value_;
    std::exception_ptr exception_;

    Generator get_return_object()
    {
      std::cout << "Generator -> promise_type -> get_return_object() ... " << "\n";
      return Generator(handle_type::from_promise(*this));
    }
    std::suspend_always initial_suspend() { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void unhandled_exception() { exception_ = std::current_exception(); } // saving
                                                                          // exception

    template<std::convertible_to<T> From> // C++20 concept
    std::suspend_always yield_value(From&& from)
    {
      std::cout << "Generator -> promise_type -> yield_value(value:" << value_ << ") ... " << "\n";
      value_ = std::forward<From>(from); // caching the result in promise
      return {};
    }
    void return_void() {}
  };

  handle_type h_;

  Generator(handle_type h) : h_(h) {
    std::cout << "Generator() constructor" << "\n";
  }
  ~Generator() { 
    h_.destroy();
    std::cout << "~Generator() destructor" << "\n";
  }
  explicit operator bool()
  {
    std::cout << "Generator -> explicit operator bool() ... " << "\n";
    fill(); // The only way to reliably find out whether or not we finished coroutine,
            // whether or not there is going to be a next value generated (co_yield)
            // in coroutine via C++ getter (operator () below) is to execute/resume
            // coroutine until the next co_yield point (or let it fall off end).
            // Then we store/cache result in promise to allow getter (operator() below
            // to grab it without executing coroutine).
    return !h_.done();
  }
  T operator()()
  {
    std::cout << "Generator -> T operator()() -> fill() || full_ = false ... " << "\n";
    fill();
    full_ = false; // we are going to move out previously cached
                    // result to make promise empty again
    return std::move(h_.promise().value_);
  }
 
private:
  bool full_ = false;

  void fill()
  {
    std::cout << "Generator -> fill() ... " << "\n";
    if (!full_)
    {
      h_();
      if (h_.promise().exception_)
          std::rethrow_exception(h_.promise().exception_);
      // propagate coroutine exception in called context

      std::cout << "Generator -> fill() -> h_() || full_ = true ... " << "\n";
      full_ = true;
    }
  }
};
 
Generator<std::uint64_t>
fibonacci_sequence(unsigned n)
{
  if (n == 0)
    co_return;

  if (n > 94)
    throw std::runtime_error("Too big Fibonacci sequence. Elements would overflow.");

  co_yield 0;

  if (n == 1)
    co_return;

  co_yield 1;

  if (n == 2)
    co_return;

  std::uint64_t a = 0;
  std::uint64_t b = 1;

  for (unsigned i = 2; i < n; ++i)
  {
    std::uint64_t s = a + b;
    std::cout << "fibonacci_sequence() -> for -> co_yield(s:" << s << ", a: " << a << ", b: " << b << ") ... " << "\n";
    co_yield s;
    a = b;
    b = s;
  }
}
 
int main()
{
  std::cout << "main start ..." << "\n";
  try
  {
    std::cout << "fibonacci_sequence(10) ... " << "\n";
    auto gen = fibonacci_sequence(10); // max 94 before uint64_t overflows

    for (int j = 0; gen; ++j) {
      std::cout << "fib(" << j << ")=" << gen() << '\n';
    }
  }
  catch (const std::exception& ex)
  {
    std::cerr << "Exception: " << ex.what() << '\n';
  }
  catch (...)
  {
    std::cerr << "Unknown exception.\n";
  }
  std::cout << "main end" << "\n";
}