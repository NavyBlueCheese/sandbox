#include <iostream>
#include <iomanip>
#include <random>
#include <cmath>
#include <chrono>

double norm_cdf(double x) {
    return 0.5 * std::erfc(-x / std::sqrt(2.0));
}

double black_scholes_call(double S, double K, double r, double sigma, double T) {
    double d1 = (std::log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));
    double d2 = d1 - sigma * std::sqrt(T);
    return S * norm_cdf(d1) - K * std::exp(-r * T) * norm_cdf(d2);
}

struct Result {
    double price;
    double std_error;
    double seconds;
};

Result mc_european(double S0, double K, double r, double sigma, double T,
                   long n_paths, unsigned seed = 42) {
    auto t0 = std::chrono::high_resolution_clock::now();

    std::mt19937_64 rng(seed);
    std::normal_distribution<double> gauss(0.0, 1.0);

    double drift = (r - 0.5 * sigma * sigma) * T;
    double diff = sigma * std::sqrt(T);

    double sum = 0.0, sum_sq = 0.0;
    for (long i = 0; i < n_paths; ++i) {
        double z = gauss(rng);
        doub payoff;
        sum_sq += payoff * payoff;
    }

    double mean = sum / n_paths;
    double var = (sum_sq / n_paths) - mean * mean;
    double disc = std::exp(-r * T);

    auto t1 = std::chrono::high_resolution_clock::now();
    double secs =le ST = S0 * std::exp(drift + diff * z);
        double payoff = std::max(ST - K, 0.0);
        sum += std::chrono::duration<double>(t1 - t0).count();

    return { disc * mean, disc * std::sqrt(var / n_paths), secs };
}

Result mc_asian(double S0, double K, double r, double sigma, double T,
                int n_steps, long n_paths, unsigned seed = 42) {
    auto t0 = std::chrono::high_resolution_clock::now();

    std::mt19937_64 rng(seed);
    std::normal_distribution<double> gauss(0.0, 1.0);

    double dt = T / n_steps;
    double drift = (r - 0.5 * sigma * sigma) * dt;
    double diff = sigma * std::sqrt(dt);

    double sum = 0.0, sum_sq = 0.0;
    for (long i = 0; i < n_paths; ++i) {
        double S = S0;
        double running_total = 0.0;

        for (int step = 0; step < n_steps; ++step) {
            S *= std::exp(drift + diff * gauss(rng));
            running_total += S;
        }

        double average = running_total / n_steps;
        double payoff = std::max(average - K, 0.0);
        sum += payoff;
        sum_sq += payoff * payoff;
    }

    double mean = sum / n_paths;
    double var = (sum_sq / n_paths) - mean * mean;
    double disc = std::exp(-r * T);

    auto t1 = std::chrono::high_resolution_clock::now();
    double secs = std::chrono::duration<double>(t1 - t0).count();

    return { disc * mean, disc * std::sqrt(var / n_paths), secs };
}

int main() {
    const double S0 = 100.0;
    const double K = 100.0;
    const double r = 0.05;
    const double sigma = 0.20;
    const double T = 1.0;

    const long n_paths = 2'000'000;
    const int n_steps = 252;

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Monte Carlo option pricing\n";
    std::cout << "S0=" << S0 << "  K=" << K << "  r=" << r
              << "  vol=" << sigma << "  T=" << T << "\n";
    std::cout << "paths=" << n_paths << "\n\n";

    double exact = black_scholes_call(S0, K, r, sigma, T);
    Result eu = mc_european(S0, K, r, sigma, T, n_paths);

    std::cout << "European call\n";
    std::cout << "  Black-Scholes exact : " << exact << "\n";
    std::cout << "  Monte Carlo estimate: " << eu.price
              << "  +/- " << 1.96 * eu.std_error << "\n";
    std::cout << "  Absolute difference : " << std::abs(eu.price - exact) << "\n";
    std::cout << "  Elapsed             : " << eu.seconds << " s\n";

    bool ok = std::abs(eu.price - exact) < 1.96 * eu.std_error;
    std::cout << "  Validation          : " << (ok ? "within 95% interval"
                                                  : "OUTSIDE interval") << "\n\n";

    Result as = mc_asian(S0, K, r, sigma, T, n_steps, n_paths / 10);

    std::cout << "Asian call, arithmetic average\n";
    std::cout << "  Monte Carlo estimate: " << as.price
              << "  +/- " << 1.96 * as.std_error << "\n";
    std::cout << "  Elapsed             : " << as.seconds << " s\n";

    return 0;
}
