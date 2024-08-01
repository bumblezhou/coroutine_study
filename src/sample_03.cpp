#include <coroutine>
#include <cstdint>
#include <exception>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <memory>
 
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
      std::cout << "Generator -> promise_type -> get_return_object(value:" << value_ << ")" << "\n";
      return Generator(handle_type::from_promise(*this));
    }
    std::suspend_always initial_suspend() { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void unhandled_exception() { exception_ = std::current_exception(); } // saving exception

    template<std::convertible_to<T> From> // C++20 concept
    std::suspend_always yield_value(From&& from)
    {
      std::cout << "Generator -> promise_type -> yield_value(value:" << value_ << ", from: " << std::forward<From>(from) << ")" << "\n";
      value_ = std::forward<From>(from); // caching the result in promise
      return {};
    }
    void return_void() {}
  };

  handle_type h_;

  Generator(handle_type h) : h_(h) {
    std::cout << "Generator constructor" << "\n";
  }
  ~Generator() { 
    h_.destroy();
    std::cout << "Generator destructor" << "\n";
  }
  explicit operator bool()
  {
    std::cout << "Generator -> Before explicit operator bool() ==> value: " << h_.promise().value_ << ", " << "full_: " << full_ << " ... " << "\n";
    std::cout << "fill() \n";
    fill(); // The only way to reliably find out whether or not we finished coroutine,
            // whether or not there is going to be a next value generated (co_yield)
            // in coroutine via C++ getter (operator () below) is to execute/resume
            // coroutine until the next co_yield point (or let it fall off end).
            // Then we store/cache result in promise to allow getter (operator() below
            // to grab it without executing coroutine).
    std::cout << "Generator -> After  explicit operator bool() ==> value: " << h_.promise().value_ << ", " << "full_: " << full_ << " ... " << "\n";
    return !h_.done();
  }
  T operator()()
  {
    std::cout << "Generator -> Before T operator()() ==> full: " << full_ << ", value: " << h_.promise().value_ << "\n";
    std::cout << "fill() \n";
    fill();
    std::cout << "full_ = false \n";
    full_ = false; // we are going to move out previously cached result to make promise empty again
    std::cout << "return std::move(h_.promise().value_) \n";
    std::cout << "Generator -> After  T operator()() ==> full: " << full_ << ", value: " << h_.promise().value_ << "\n";
    return std::move(h_.promise().value_);
  }
 
private:
  bool full_ = false;

  void fill()
  {
    std::cout << "Generator -> Before fill() ==> full: " << full_ << ", value: " << h_.promise().value_ << "\n";
    if (!full_)
    {
      std::cout << "h_() \n";
      h_();
      if (h_.promise().exception_)
      {
        std::rethrow_exception(h_.promise().exception_);
      }
      // propagate coroutine exception in called context
      std::cout << "full_ = true \n";
      full_ = true;
    }
    std::cout << "Generator -> After  fill() ==> full: " << full_ << ", value: " << h_.promise().value_ << "\n";
  }
};
 
Generator<std::uint64_t>
fibonacci_sequence(uint32_t n)
{
  if (n == 0)
    co_return;

  if (n > 94)
    throw std::runtime_error("Too big Fibonacci sequence. Elements would overflow.");

  std::cout << "fibonacci_sequence(" << n << ") -> co_yield 0 " << "\n";
  co_yield 0;

  if (n == 1) {
    std::cout << "fibonacci_sequence(" << n << ") -> co_return " << "\n";
    co_return;
  }

  std::cout << "fibonacci_sequence(" << n << ") -> co_yield 1 " << "\n";
  co_yield 1;

  if (n == 2)
    co_return;

  std::uint64_t a = 0;
  std::uint64_t b = 1;

  std::cout << "fibonacci_sequence(" << n << ") -> for loop -> a: " << a << ", b: " << b << " ... " << "\n";
  for (uint32_t i = 2; i < n; ++i)
  {
    std::uint64_t s = a + b;
    std::cout << "fibonacci_sequence(" << n << ") -> for(i:" << i << ") -> a: " << a << ", b: " << b << " => co_yield() s:" << s << "\n";
    co_yield s;
    a = b;
    b = s;
  }
}
 
Generator<double_t>
newton_method_for_pi(double_t initial_guess) {
  double_t x0 = 3.0; // Initial guess
  double_t epsilon = 1e-8; // Convergence criteria
  double_t x = x0;
  while (true) {
    double_t fx = std::sin(x);
    double_t fpx = std::cos(x);
    double_t x1 = x - (fx / fpx);

    if (std::abs(x1 - x) < epsilon) {
      // return x1;
      std::cout << "############co_yield " << std::setprecision(15) << x1 << ";#############\n";
      co_yield x1;
      std::cout << "break; \n";
      break;
    }

    x = x1;
    std::cout << "############co_yield " << std::setprecision(15) << x << ";#############\n";
    co_yield x;
  }
}

double_t machin_like_formula_pi() {
    double_t pi = 4 * (4 * std::atan(1.0 / 5.0) - std::atan(1.0 / 239.0));
    return pi;
}

int main()
{
  std::cout << "main start ..." << "\n";
  try
  {
    std::cout << "main call fibonacci_sequence(10) ... " << "\n";
    auto fib_gen = fibonacci_sequence(10); // max 94 before uint64_t overflows
    std::cout << "main for loop ..." << "\n";
    for (int j = 0; fib_gen; ++j) {
      std::cout << "main for(" << j << ") fib_gen() => " << fib_gen() << '\n';
    }

    std::cout << "\n\n";
    std::cout << "main call newton_method_for_pi(3.0) ... " << "\n";
    auto pi_gen = newton_method_for_pi(3.0);
    std::cout << "main for loop ..." << "\n";
    for (int j = 0; pi_gen; ++j) {
      std::cout << "main for(" << j << ") pi_gen() => " << pi_gen() << '\n';
    }

    std::cout << "\n\n";
    double_t pi_02 = machin_like_formula_pi();
    std::cout << "machin_like_formula_pi: " << std::setprecision(12) << pi_02 << std::endl;
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