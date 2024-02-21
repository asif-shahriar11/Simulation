import random
import matplotlib.pyplot as plt

def is_selection_successful(n, s, m):
    """
    parameters:
    n: number of candidates
    s: top s candidates are considered successful
    m: number of candidates to create a standard before the interview
    return: True if the chosen candidate belongs to the top s candidates, False otherwise
    """
    candidates = list(range(1, n + 1))  # Rank. Best : 1, Worst : n
    random.shuffle(candidates)

    if m == 0: return False  # no standard to compare

    chosen = -1
    standard = min(candidates[:m])  # setting the standard to compare
    for candidate in candidates[m:]:
        if candidate < standard: 
            chosen = candidate # found a better candidate
            break
    else: chosen = n # no candidate met the standard
    
    return chosen <= s # True if the chosen candidate belongs to the top s candidates


def secretary_simulation(n, s, m, num_trials):
    """
    parameters:
    n: number of candidates
    s: top s candidates are considered successful
    m: number of candidates to create a standard before the interview
    num_trials: number of trials to run the simulation
    return: the probability of choosing a successful candidate
    """
    successful_trials = 0
    for _ in range(num_trials):
        if is_selection_successful(n, s, m): successful_trials += 1
    return successful_trials / num_trials


def plot(n, s, results):
    """Plots the success rate for given n and s"""
    plt.plot(range(n), results, label=f"n = {n}, s = {s}")
    plt.xlabel("Sample Size (m)")
    plt.ylabel("Success Rate")
    plt.title("Secretary Problem Success Rate vs. Sample Size")
    plt.ylim(0.0, 1.0)  # Fix the y-axis scale from 0.0 to 1.0
    plt.grid(True)
    plt.legend()
    plt.savefig(f"secretary_problem_n_{n}_s_{s}.png")  # Save the figure before showing it
    plt.show()


# demonstration
if __name__ == "__main__":
    n = 100  # number of candidates
    success_criteria = [1, 3, 5, 10]  # top s candidates are considered successful
    num_trials = 10000
    for s in success_criteria:
        results = []
        for m in range(n):  results.append(secretary_simulation(n, s, m, num_trials))
        plot(n, s, results)
    print("Done")

