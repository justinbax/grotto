#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>

#define NODE_COUNT 7 // Number of nodes (grottos)
#define MAX_EVENTS 3 // Maximum number of events (pokemons) per node
#define ITERATIONS 50000 // Number of simulations; increase for better accuracy
// For reference, 100 000 iterations on a Ryzen 7 3700X @ 4.19GHz boosted and 3000MHz DDR4 takes about 25s, with an estimated precision of about 0.1%.
// Multithreading would extremely benefit the speed of this program and would be very easy to use in this context, however given huge precision is not needed and 10 000 iterations is honestly fine, I decided it's not worthwhile.

#define MAX_TOTAL 127 // Amount of lost time when travelling to all non-set nodes once beyond which the number of 256 steps to reload nodes is not affected
#define LAST_NODE_MULTIPLIER 18 // Amount of steps per second when realoading nodes 
#define EVENT_ROLL_PROB 50 // Probability for a node to reload for each 256 steps in 1/10 %

typedef struct {
	int probabilities[MAX_EVENTS]; // Probabilities for each event happening, in 1/10 %
	int set; // Bitmap of the status (cleared or not) of events; 1 for a non-cleared event with probability > 0
	bool test; // Does this node has a non-zero number of events with probability > 0
	bool skip; // Should this node be ignored (travelled over)
} node;

void initNode(node *newNode, const int *probabilities) {
	newNode->set = 0;
	newNode->skip = false;

	// Instead of storing probabilities themselves, we store the sum of all previous probabilities plus the current one in order to test them all with a single random number
	int probabilitiesTotal = 0;
	for (int i = 0; i < MAX_EVENTS; i++) {
		probabilitiesTotal += probabilities[i];
		newNode->probabilities[i] = probabilitiesTotal;
		newNode->set |= (probabilities[i] > 0) << i;
	}
	newNode->test = newNode->set;
}

void searchNode(node *toSearch, int reloadTries) {
	bool success = false;
	for (int i = 0; i < reloadTries; i++) {
		if (rand() % 1000 < EVENT_ROLL_PROB) success = true;
	}
	if (!success) return;

	int result = rand() % 1000;
	for (int i = 0; i < MAX_EVENTS; i++) {
		if (result < toSearch->probabilities[i]) {
			toSearch->set &= ~(1 << i);
			break;
		}
	}
}

long int setNode(node *nodes, int start, const int count, const int *gains, const int *losses, const int *setsPerLoss) {
	// Base case : nullptrs or all nodes must either be skipped or not be tested
	if (nodes == NULL || gains == NULL || losses == NULL) return 0;
	for (int i = 0; i < count; i++) {
		if (nodes[i].test && !nodes[i].skip) break;
		if (i == count - 1) return 0;
	}

	// Goes through a simulation of a single cycle (travelling until at the same start point) in order to determine the amount of reload (biking) time needed from the amount of time loss
	// This must handle a start location that must be skipped, so we can't just test for to == start or from == start
	long int total = 0;
	int from = start;
	int to = from;
	int nodesChecked = 0;
	while (true) {
		do {
			to++;
			nodesChecked++;
			if (to == count) to = 0;
		} while (nodes[to].skip);
		if (nodesChecked >= count) break;
		total += losses[from * count + to];
		from = to;
	}
	int lastNodeBonus = setsPerLoss[total > MAX_TOTAL ? MAX_TOTAL : total] * LAST_NODE_MULTIPLIER;
	
	// Resets the simulation and computes the real total, adding losses, subtracting gains, and adding the previously computed bonus each time the last node is travelled over (directly or not)
	total = 0;
	from = start;
	to = from;
	while (true) {
		do {
			to++;
			if (to == count) to = 0;
			else if (to == count - 1) total += lastNodeBonus;
		} while (nodes[to].skip);

		total += losses[from * count + to];
		total -= gains[from * count + to];
		searchNode(&nodes[to], setsPerLoss[total > MAX_TOTAL ? MAX_TOTAL : total]);
		from = to;
		if (!nodes[to].set && nodes[to].test) break;
	}
	
	nodes[from].skip = true;
	// We can represent the final total as the total given to clear one node plus the total given to clear all other nodes.
	// Because "clearing all other nodes" is the same as "clearing all nodes" but setting the cleared node's skip flag (which we already did),
	// we can make this function recursive by only computing the total for clearing one node and calling itself, skipping the cleared node.
	return total + setNode(nodes, from, count, gains, losses, setsPerLoss);
}

int main() {
	// haha magic number go weeeee
	const int losses[NODE_COUNT * NODE_COUNT] = {
		0, 10, 19, 22, 35, 51, 0,
		0, 0, 16, 15, 35, 51, 0,
		0, 0, 0, 22, 39, 51, 7,
		0, 0, 0, 0, 20, 51, 0,
		0, 0, 0, 0, 0, 51, 0,
		0, 0, 0, 0, 0, 0, 14,
		10, 10, 17, 10, 17, 51, 0
	};

	const int gains[NODE_COUNT * NODE_COUNT] = {
		0, 15, 16, 14, 18, 21, 0,
		0, 0, 5, 21, 18, 21, 0,
		0, 0, 0, 19, 17, 21, 0,
		0, 0, 0, 0, 18, 21, 0,
		0, 0, 0, 0, 0, 21, 0,
		0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 3, 0, 0
	};

	const int events[NODE_COUNT][MAX_EVENTS] = {
		{10, 20, 20},
		{40, 150, 0},
		{10, 40, 150},
		{10, 30, 150},
		{10, 40, 40},
		{40, 0, 0},
		{0, 0, 0}
	};

	srand(time(NULL));

	node nodes[NODE_COUNT];

	// We (uglily) create an array to map the number of seconds lost to the number of 256 steps to do for optimally reloading nodes instead of creating a function in order to access it in constant time
	// The creation of the array makes the whole process O(n) even if access time is O(1), but in practice, it's still way better than the O(n) of a function called over and over again
	int setsPerLoss[MAX_TOTAL + 1];
	for (int i = 0; i <= 15; i++) setsPerLoss[i + 0] = 5;
	for (int i = 0; i <= 5; i++) setsPerLoss[i + 16] = 6;
	for (int i = 0; i <= 7; i++) setsPerLoss[i + 22] = 7;
	for (int i = 0; i <= 9; i++) setsPerLoss[i + 30] = 8;
	for (int i = 0; i <= 9; i++) setsPerLoss[i + 40] = 9;
	for (int i = 0; i <= 11; i++) setsPerLoss[i + 50] = 10;
	for (int i = 0; i <= 13; i++) setsPerLoss[i + 62] = 11;
	for (int i = 0; i <= 14; i++) setsPerLoss[i + 76] = 12;
	for (int i = 0; i <= 16; i++) setsPerLoss[i + 91] = 13;
	for (int i = 0; i <= 18; i++) setsPerLoss[i + 108] = 14;
	setsPerLoss[MAX_TOTAL] = 15;

	long long int total = 0;
	for (int i = 0; i < ITERATIONS; i++) {
		for (int j = 0; j < NODE_COUNT; j++) {
			initNode(&nodes[j], events[j]);
		}
		// Example : uncomment this to simulate doing only grottos 0-4 (node 6 is the place where the reloading takes place; it is not a grotto and cannot be tested)
		// nodes[5].skip = true;
		total += setNode(nodes, NODE_COUNT - 1, NODE_COUNT, gains, losses, setsPerLoss);
	}
	printf("%f\n", (float)total / ITERATIONS);
	return 0;
}