import sys
import time
import datetime
import timeit
import random

n = 1000
m = 50
threshold = 0.5

def make_fingerprint(file_name):
	new_fp = [[0 for x in range(m)] for y in range(n)]

	for i in range(1, m+1):
		for j in range(1, n+1):

			# Currently using the time.perf_counter_ns() which according to documentation is the most
			# precise Python timer for tracking small durations

			# Python also allows for selecting the same Clock IDs as in the C++ clock_gettime() function
			# (CLOCK_REALTIME, CLOCK_MONOTONIC, CLOCK_PROCESS_CPUTIME_ID), though on GCP the CLOCK_REALTIME
			# seems to run into the same issue as in the C++ version with still appealing to the TSC
			start_time = time.perf_counter_ns()
			for k in range(j):
				hash(j)
			#random.randbytes(j)
			end_time = time.perf_counter_ns()

			log_time = end_time - start_time

			new_fp[j-1][i-1] = log_time


			# Alternative approach using the timeit library

			#log_time = timeit.timeit('hash("input")', number=j)
			#new_fp[j-1][i-1] = log_time

	if file_name != None:
		with open(file_name, "w") as fp_file:
			for i in range (m):
				line = ""
				for j in range(n):
					line += str(new_fp[j][i]) + " "
				fp_file.write(line + "\n")

	return new_fp


def read_fingerprint(file_name):
	read_fp = [[0 for x in range(m)] for y in range(n)]

	with open(file_name, "r") as fp_file:
		i = 0
		for line in fp_file:
			entries = line.split()
			j = 0
			for entry in entries:
				read_fp[j][i] = float(entry)
				j += 1
			i += 1

	return read_fp


def compute_mode(lst):
	return max(set(lst), key=lst.count)


def get_num_coincidences(fp1, fp2):
	num_coin = 0
	fp1_modes = [0 for x in range(n)]

	for i in range(n):
		fp1_modes[i] = compute_mode(fp1[i])

	for i in range(n):
		check = False
		j = 0
		while j < m and not check:
			if fp1_modes[i] == fp2[i][j]:
				num_coin = num_coin + 1
				check = True
			else:
				j = j + 1

	return num_coin


def match_fingerprints(fp1, fp2):
	num_coin = get_num_coincidences(fp1, fp2)
	num_coin += get_num_coincidences(fp2, fp1)

	print(str(num_coin) + "/" + str(2 * n))
	return num_coin >= threshold * 2 * n


def main():
	if len(sys.argv) == 2 and sys.argv[1] != "-cmp":
		make_fingerprint(sys.argv[1])
	elif len(sys.argv) == 3 and sys.argv[1] != "-cmp" and sys.argv[2] == "-cmp":
		new_fp = make_fingerprint(None)
		old_fp = read_fingerprint(sys.argv[1])

		if match_fingerprints(new_fp, old_fp):
			print("fingerprint match")
		else:
			print("no match")
	else:
		print("usage: python3 fingerprint.py <fingerprint_filename> [-cmp]")

if __name__ == '__main__':
	main()



