import re
import numpy as np
import sys

def main():
	results = [[0 for x in range(60)] for y in range(60)]

	f = open(sys.argv[1], "r")
	text = f.readlines()

	for line in text:
		m = re.match(r"Testing fingerprint (\d+) on machine (\d+) with results: (\d+)/(\d+)", line)

		if m:
			fp_num = int(m.group(1))
			client_num = int(m.group(2))
			pass_count = int(m.group(3))
			test_count = int(m.group(4))

			results[client_num][fp_num] = pass_count
		

	print("Fingerprint results matrix:\n", np.matrix(results))

	matches = np.diagonal(results)
	stability_rate = np.mean(matches) / test_count
	print("Stability rate:\n", stability_rate)

	collisions = np.matrix(results) - np.diag(matches)
	collision_rate = np.mean(collisions) / test_count
	print("Collision rate:\n", collision_rate)

if __name__ == '__main__':
	main()