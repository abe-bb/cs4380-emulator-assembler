#include <iostream>
#include <string>

unsigned int mod(unsigned dividend, unsigned divisor) {
  return dividend % divisor;
}

bool isPrime(unsigned int num) {
  if (num < 2) {
    return false;
  }

  if (num == 2) {
    return true;
  }

  if (mod(num, 2) == 0) {
    return false;
  }

  unsigned int div = num - 2;
  while (div != 1) {
    if (mod(num, div) == 0) {
      return false;
    }

    div -= 2;
  }
  return true;
}

int main() {
  std::string prompt = "Welcome to the Prime Number Generator.\n\nThis program searches for and displays the first 20 prime numbers greater than or equal to a user provided lower bound.\n\nPlease enter a lower bound: ";
  std::string out1 = "The first 20 prime numbers greater than or equal to ";
  std::string out2 = " are:\n";
  unsigned int primes[20] = {0};
  unsigned int prime_index = 0;
  unsigned int lower_bound = 0;

  std::cout << prompt;
  std::cin >> lower_bound;

  unsigned int i;

  // check for 2 
  if (lower_bound <= 2) {
    primes[prime_index] = 2;
    prime_index += 1;
    i = 3;
  }
  // check for even lower bound
  else if (mod(lower_bound, 2) == 0) {
    i = lower_bound + 1;
  }
  else {
    i = lower_bound;
  }

  // calculate and store 20 primes
  while (prime_index < 20) {
    if (isPrime(i)) {
      primes[prime_index] = i;
      prime_index += 1;
    }

    i += 2;
  }
  

  std::cout << out1 << lower_bound << out2;
  for (auto i = 0; i < 20; i++) {
    std::cout << primes[i] << "\n";
  }
  
  return 0;  
}
