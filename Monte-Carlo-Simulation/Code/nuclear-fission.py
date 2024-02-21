import random

class Fission:
    def __init__(self, num_max_neutrons, num_generations, p_1, common_ratio):
        self.num_max_neutrons = num_max_neutrons
        self.num_generations = num_generations
        # Calculate probabilities: p_i = probability of i new neutrons
        self.probabilities = [0] * (num_max_neutrons + 1)
        for i in range(1, num_max_neutrons + 1): self.probabilities[i] = p_1 * common_ratio ** (i - 1)
        self.probabilities[0] = 1 - sum(self.probabilities)  # Adjust for probability of 0 new neutrons
        # results[i][j] = probability of having j neutrons in generation i
        self.results = [[0 for _ in range(num_max_neutrons + 2)] for _ in range(num_generations)] # +2 to include 0 and num_max_neutrons + 1


    def _random(self):
        """Chooses a random value based on defined probabilities"""
        return random.choices(range(self.num_max_neutrons + 1), weights=self.probabilities, k=1)[0]
    

    def simulate(self, num_iterations=10000):
        """Simulates the process for the specified number of iterations."""
        for gen in range(self.num_generations):
            count = [0] * num_iterations  # count[i] = final neutron count for simulation i
            for simulation in range(num_iterations):
                neutron_count = 1  # start with 1 neutron
                for _ in range(gen + 1):
                    new_neutron_count = 0 # count of new neutrons for this generation
                    for _ in range(neutron_count):  new_neutron_count += self._random() # add new neutrons based on probabilities
                    neutron_count = new_neutron_count
                count[simulation] = neutron_count 
            
            # Update results based on counts, adjusting for max neutron threshold
            for neutron_count in count:
                if neutron_count <= self.num_max_neutrons + 1: self.results[gen][neutron_count] += 1

            # Normalize the results for this generation to probabilities
            self.results[gen] = [c / num_iterations for c in self.results[gen]]
        
        self._save_probabilities()


    def _save_probabilities(self):
        """Save the probabilities to a text file in the specified format."""
        with open("fission_output.txt", "w") as file:
            for gen, probs in enumerate(self.results, start=1):
                file.write(f"Generation-{gen}:\n")
                for j, prob in enumerate(probs):
                    file.write(f"p[{j}] = {prob:.4f}\n")
                file.write("\n")  # Add a newline for spacing between generations

# demonstration
if __name__ == "__main__":
    num_generations = 10
    max_new_neutrons = 3
    p_1 = 0.2126
    common_ratio = 0.5893
    iterations = 10000

    fission = Fission(max_new_neutrons, num_generations, p_1, common_ratio)
    fission.simulate(iterations)
