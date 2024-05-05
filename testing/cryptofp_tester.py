import paramiko
import re
import sys
import threading
import time
from scp import SCPClient
import numpy as np

aws_vms = [
	"ec2-3-137-166-242.us-east-2.compute.amazonaws.com",
	"ec2-18-223-209-169.us-east-2.compute.amazonaws.com",
	"ec2-18-118-31-95.us-east-2.compute.amazonaws.com",
	"ec2-18-119-17-97.us-east-2.compute.amazonaws.com",
	"ec2-3-140-195-58.us-east-2.compute.amazonaws.com",
	"ec2-18-191-29-4.us-east-2.compute.amazonaws.com",
	"ec2-18-117-70-174.us-east-2.compute.amazonaws.com",
	"ec2-18-224-53-175.us-east-2.compute.amazonaws.com",
	"ec2-3-139-88-110.us-east-2.compute.amazonaws.com",
	"ec2-18-118-254-6.us-east-2.compute.amazonaws.com", # VM 10
	"ec2-18-191-124-49.us-east-2.compute.amazonaws.com",
	"ec2-18-224-70-42.us-east-2.compute.amazonaws.com",
	"ec2-3-139-70-132.us-east-2.compute.amazonaws.com",
	"ec2-3-142-124-62.us-east-2.compute.amazonaws.com",
	"ec2-3-129-209-216.us-east-2.compute.amazonaws.com",
	"ec2-3-147-79-245.us-east-2.compute.amazonaws.com",
	"ec2-3-145-36-128.us-east-2.compute.amazonaws.com",
	"ec2-3-15-219-179.us-east-2.compute.amazonaws.com",
	"ec2-18-218-40-50.us-east-2.compute.amazonaws.com",
	"ec2-3-144-250-235.us-east-2.compute.amazonaws.com", # VM 20
	"ec2-3-145-81-220.us-east-2.compute.amazonaws.com",
	"ec2-3-139-82-96.us-east-2.compute.amazonaws.com",
	"ec2-3-137-182-185.us-east-2.compute.amazonaws.com",
	"ec2-18-119-119-76.us-east-2.compute.amazonaws.com",
	"ec2-3-137-163-201.us-east-2.compute.amazonaws.com",
	"ec2-3-21-105-64.us-east-2.compute.amazonaws.com",
	"ec2-18-221-20-35.us-east-2.compute.amazonaws.com",
	"ec2-18-119-104-19.us-east-2.compute.amazonaws.com",
	"ec2-3-138-175-245.us-east-2.compute.amazonaws.com",
	"ec2-3-133-108-144.us-east-2.compute.amazonaws.com", # VM 30
	"ec2-18-191-176-89.us-east-2.compute.amazonaws.com",
	"ec2-3-23-92-187.us-east-2.compute.amazonaws.com",
	"ec2-3-145-202-240.us-east-2.compute.amazonaws.com",
	"ec2-3-137-190-89.us-east-2.compute.amazonaws.com",
	"ec2-18-225-234-179.us-east-2.compute.amazonaws.com",
	"ec2-3-148-105-17.us-east-2.compute.amazonaws.com",
	"ec2-18-116-20-202.us-east-2.compute.amazonaws.com",
	"ec2-3-16-135-197.us-east-2.compute.amazonaws.com",
	"ec2-18-225-235-102.us-east-2.compute.amazonaws.com",
	"ec2-18-119-143-241.us-east-2.compute.amazonaws.com", # VM 40
	"ec2-18-118-128-254.us-east-2.compute.amazonaws.com",
	"ec2-18-116-118-52.us-east-2.compute.amazonaws.com",
	"ec2-18-222-167-128.us-east-2.compute.amazonaws.com",
	"ec2-3-15-225-237.us-east-2.compute.amazonaws.com",
	"ec2-3-15-143-233.us-east-2.compute.amazonaws.com",
	"ec2-3-133-107-147.us-east-2.compute.amazonaws.com",
	"ec2-3-22-130-36.us-east-2.compute.amazonaws.com",
	"ec2-3-138-114-167.us-east-2.compute.amazonaws.com",
	"ec2-3-22-242-151.us-east-2.compute.amazonaws.com",
	"ec2-18-119-159-32.us-east-2.compute.amazonaws.com", # VM 50
	"ec2-3-147-193-70.us-east-2.compute.amazonaws.com",
	"ec2-3-138-60-67.us-east-2.compute.amazonaws.com",
	"ec2-3-147-57-189.us-east-2.compute.amazonaws.com",
	"ec2-52-15-162-111.us-east-2.compute.amazonaws.com",
	"ec2-3-145-65-133.us-east-2.compute.amazonaws.com",
	"ec2-3-136-23-69.us-east-2.compute.amazonaws.com",
	"ec2-18-225-234-76.us-east-2.compute.amazonaws.com",
	"ec2-18-222-107-109.us-east-2.compute.amazonaws.com",
	"ec2-3-149-230-103.us-east-2.compute.amazonaws.com",
	"ec2-18-189-192-113.us-east-2.compute.amazonaws.com" # VM 60
]

test_count = 100

#############################################
####		 SSH Helper Methods			 ####
#############################################
def create_ssh_client(server, user, key_file):
    client = paramiko.SSHClient()
    client.load_system_host_keys()
    client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    client.connect(server, username=user, key_filename=key_file)
    return client


def scp_retrieve_file(client, file_path):
	scp = SCPClient(client.get_transport())
	scp.get(file_path)
	scp.close()


def scp_send_file(client, file_path):
	scp = SCPClient(client.get_transport())
	scp.put(file_path)
	scp.close()


#############################################
####	  Virtual Machine Management     ####
#############################################
def initialize_clients():
	for i in range(len(aws_vms)):
		client = create_ssh_client(aws_vms[i], "ubuntu", "server-1-keypair.pem")
		stdin, stdout, stderr = client.exec_command("ls LicenseLocking/cryptofp-cpp/main")
		ls_result = stderr.readlines()

		if (len(ls_result) > 0):
			scp_send_file(client, "LicenseLocking-main.zip")
			client.exec_command("sudo apt-get install unzip")
			time.sleep(5)
			client.exec_command("unzip LicenseLocking-main.zip")
			stdin, stdout, stderr = client.exec_command("cd LicenseLocking/cryptofp-cpp && ./build.sh && make")
			print(stdout.readlines())
			print(stderr.readlines())
			print("Built LicenseLocking package for machine " + str(i))
		else:
			print("LicenseLocking package was already initialized for machine " + str(i))

		client.close()

def initialize_client(client_num):
	client = create_ssh_client(aws_vms[client_num], "ubuntu", "server-1-keypair.pem")

	stdin, stdout, stderr = client.exec_command("ls LicenseLocking/cryptofp-cpp/main")
	ls_result = stderr.readlines()

	if (len(ls_result) > 0):
		scp_send_file(client, "LicenseLocking-main.zip")
		client.exec_command("sudo apt-get install unzip")
		time.sleep(5)
		client.exec_command("unzip LicenseLocking-main.zip")
		stdin, stdout, stderr = client.exec_command("cd LicenseLocking/cryptofp-cpp && ./build.sh && make")
		print(stdout.readlines())
		print(stderr.readlines())
		print("Built LicenseLocking package for machine " + str(client_num))
	else:
		print("LicenseLocking package was already initialized for machine " + str(client_num))

	client.close()

def initialize_clients_threaded():
	thread_list = []

	for client_num in range(len(aws_vms)):
		t = threading.Thread(target=initialize_client, args=[client_num])
		t.daemon = True
		t.start()
		thread_list.append(t)

	for thread in thread_list:
		thread.join()


def generate_fingerprints():
	gen_times = [0 for x in range(len(aws_vms))]

	for i in range(len(aws_vms)):
		client = create_ssh_client(aws_vms[i], "ubuntu", "server-1-keypair.pem")
		fp_path = "~/fp-" + str(i)
		command_string = "cd LicenseLocking/cryptofp-cpp && sudo ./select_timer.sh hpet && time ./main " + fp_path;
		stdin, stdout, stderr = client.exec_command(command_string)

		time_output = stderr.readlines()
		time_string = "".join(time_output)

		time_match = re.search(r"(\d)m(\d{1,2})\.(\d{3})s", time_string)
		minutes = time_match.group(1)
		seconds = time_match.group(2)
		fract_seconds = time_match.group(3)
		total_time = int(minutes) * 60 + int(seconds) + int(fract_seconds) * 0.001 
		gen_times[i] = total_time

		print("Generated fingerprint for machine " + str(i))

		client.close()

	return gen_times

def generate_fingerprint(client_num, gen_times):
	client = create_ssh_client(aws_vms[client_num], "ubuntu", "server-1-keypair.pem")
	fp_path = "~/fp-" + str(client_num)
	command_string = "cd LicenseLocking/cryptofp-cpp && sudo ./select_timer.sh hpet && time ./main " + fp_path;
	stdin, stdout, stderr = client.exec_command(command_string)

	time_output = stderr.readlines()
	time_string = "".join(time_output)

	time_match = re.search(r"(\d)m(\d{1,2})\.(\d{3})s", time_string)
	minutes = time_match.group(1)
	seconds = time_match.group(2)
	fract_seconds = time_match.group(3)
	total_time = int(minutes) * 60 + int(seconds) + int(fract_seconds) * 0.001 
	gen_times[client_num] = total_time

	print("Generated fingerprint for machine " + str(client_num))

	client.close()

def generate_fingerprints_threaded():
	gen_times = [0 for x in range(len(aws_vms))]

	thread_list = []

	for client_num in range(len(aws_vms)):
		t = threading.Thread(target=generate_fingerprint, args=[client_num, gen_times])
		t.daemon = True
		t.start()
		thread_list.append(t)

	for thread in thread_list:
		thread.join()

	return gen_times



def download_fingerprints():
	for i in range(len(aws_vms)):
		client = create_ssh_client(aws_vms[i], "ubuntu", "server-1-keypair.pem")
		fp_path = "~/fp-" + str(i)
		scp_retrieve_file(client, fp_path)

		print("Downloaded fingerprint for machine " + str(i))

		client.close()

def download_fingerprint(client_num):
	client = create_ssh_client(aws_vms[client_num], "ubuntu", "server-1-keypair.pem")
	fp_path = "~/fp-" + str(client_num)
	scp_retrieve_file(client, fp_path)

	print("Downloaded fingerprint for machine " + str(client_num))

	client.close()

def download_fingerprints_threaded():
	thread_list = []

	for client_num in range(len(aws_vms)):
		t = threading.Thread(target=download_fingerprint, args=[client_num])
		t.daemon = True
		t.start()
		thread_list.append(t)

	for thread in thread_list:
		thread.join()


def upload_fingerprints():
	for client_num in range(len(aws_vms)):
		client = create_ssh_client(aws_vms[client_num], "ubuntu", "server-1-keypair.pem")
		client.exec_command("mkdir LicenseLocking/cryptofp-cpp/test_fps")

		for fp_num in range(len(aws_vms)):
			fp_string = "fp-" + str(fp_num)
			scp_send_file(client, "./" + fp_string)
			client.exec_command("mv " + fp_string + " LicenseLocking/cryptofp-cpp/test_fps")

			print("Uploaded fingerprint " + str(fp_num) + " to machine " + str(client_num))

		client.close()

def upload_fingerprints_single_client(client_num):
	client = create_ssh_client(aws_vms[client_num], "ubuntu", "server-1-keypair.pem")
	client.exec_command("mkdir LicenseLocking/cryptofp-cpp/test_fps")

	for fp_num in range(len(aws_vms)):
		fp_string = "fp-" + str(fp_num)
		scp_send_file(client, "./" + fp_string)
		client.exec_command("mv " + fp_string + " LicenseLocking/cryptofp-cpp/test_fps")

		print("Uploaded fingerprint " + str(fp_num) + " to machine " + str(client_num))

	client.close()

def upload_fingerprints_threaded():
	thread_list = []

	for client_num in range(len(aws_vms)):
		t = threading.Thread(target=upload_fingerprints_single_client, args=[client_num])
		t.daemon = True
		t.start()
		thread_list.append(t)

	for thread in thread_list:
		thread.join()


def run_synchronous_test():
	results = [[0 for x in range(len(aws_vms))] for y in range(len(aws_vms))]
	times = [[0 for x in range(len(aws_vms))] for y in range(len(aws_vms))]

	for client_num in range(len(aws_vms)):
		client = create_ssh_client(aws_vms[client_num], "ubuntu", "server-1-keypair.pem")

		for fp_num in range(len(aws_vms)):
			fp_path = "test_fps/fp-" + str(fp_num)
			test_param = "--test_count=" + str(test_count)
			command_string = "cd LicenseLocking/cryptofp-cpp && time ./taskset_test.sh --fp=" + fp_path + " " + test_param + " --stress=0"
			stdin, stdout, stderr = client.exec_command(command_string)
			cmd_output = stdout.readline()

			count_match = re.match(r'cpu 0: (\d+)/(\d+)', cmd_output)
			fail_count = count_match.group(1)
			pass_count = test_count - int(fail_count)
			results[client_num][fp_num] = pass_count

			time_output = stderr.readlines()
			time_string = "".join(time_output)

			time_match = re.search(r"(\d)m(\d{1,2})\.(\d{3})s", time_string)
			minutes = time_match.group(1)
			seconds = time_match.group(2)
			fract_seconds = time_match.group(3)
			total_time = int(minutes) * 60 + int(seconds) + int(fract_seconds) * 0.001 
			times[client_num][fp_num] = total_time

			print("Testing fingerprint " + str(fp_num) + " on machine " + str(client_num) + " with results: " + str(pass_count) + "/" + str(test_count))

		client.close()

	return results, times

def run_vm_test(client_num, results, times):
	client = create_ssh_client(aws_vms[client_num], "ubuntu", "server-1-keypair.pem")

	for fp_num in range(len(aws_vms)):
		fp_path = "test_fps/fp-" + str(fp_num)
		test_param = "--test_count=" + str(test_count)
		command_string = "cd LicenseLocking/cryptofp-cpp && time ./taskset_test.sh --fp=" + fp_path + " " + test_param + " --stress=0"
		stdin, stdout, stderr = client.exec_command(command_string)
		cmd_output = stdout.readline()

		count_match = re.match(r'cpu 0: (\d+)/(\d+)', cmd_output)
		fail_count = count_match.group(1)
		pass_count = test_count - int(fail_count)
		results[client_num][fp_num] = pass_count

		time_output = stderr.readlines()
		time_string = "".join(time_output)

		time_match = re.search(r"(\d)m(\d{1,2})\.(\d{3})s", time_string)
		minutes = time_match.group(1)
		seconds = time_match.group(2)
		fract_seconds = time_match.group(3)
		total_time = int(minutes) * 60 + int(seconds) + int(fract_seconds) * 0.001 
		times[client_num][fp_num] = total_time

		print("Testing fingerprint " + str(fp_num) + " on machine " + str(client_num) + " with results: " + str(pass_count) + "/" + str(test_count))

	client.close()

def run_threaded_test():
	results = [[0 for x in range(len(aws_vms))] for y in range(len(aws_vms))]
	times = [[0 for x in range(len(aws_vms))] for y in range(len(aws_vms))]

	thread_list = []

	for client_num in range(len(aws_vms)):
		t = threading.Thread(target=run_vm_test, args=[client_num, results, times])
		t.daemon = True
		t.start()
		thread_list.append(t)

	for thread in thread_list:
		thread.join()

	return results, times

def run_stability_vm_test(client_num, results, times):
	client = create_ssh_client(aws_vms[client_num], "ubuntu", "server-1-keypair.pem")

	fp_path = "test_fps/fp-" + str(client_num)
	test_param = "--test_count=" + str(test_count)
	command_string = "cd LicenseLocking/cryptofp-cpp && time ./taskset_test.sh --fp=" + fp_path + " " + test_param + " --stress=1"
	stdin, stdout, stderr = client.exec_command(command_string)
	cmd_output = stdout.readline()

	count_match = re.match(r'cpu 0: (\d+)/(\d+)', cmd_output)
	fail_count = count_match.group(1)
	pass_count = test_count - int(fail_count)
	results[client_num] = pass_count

	time_output = stderr.readlines()
	time_string = "".join(time_output)

	time_match = re.search(r"(\d)m(\d{1,2})\.(\d{3})s", time_string)
	minutes = time_match.group(1)
	seconds = time_match.group(2)
	fract_seconds = time_match.group(3)
	total_time = int(minutes) * 60 + int(seconds) + int(fract_seconds) * 0.001 
	times[client_num] = total_time

	print("Testing fingerprint " + str(client_num) + " on machine " + str(client_num) + " with results: " + str(pass_count) + "/" + str(test_count))

	client.close()

def run_threaded_stability_test():
	results = [0 for x in range(len(aws_vms))]
	times = [0 for x in range(len(aws_vms))]

	thread_list = []

	for client_num in range(len(aws_vms)):
		t = threading.Thread(target=run_stability_vm_test, args=[client_num, results, times])
		t.daemon = True
		t.start()
		thread_list.append(t)

	for thread in thread_list:
		thread.join()

	return results, times


#############################################
####		   Main Execution			 ####
#############################################
def main():
	initialize_clients_threaded()
	gen_times = generate_fingerprints_threaded()
	time.sleep(1)
	download_fingerprints()
	upload_fingerprints_threaded()
	results, times = run_threaded_test()
	print("Fingerprint generation times:\n", gen_times)
	print("Fingerprint results matrix:\n", results)
	print("Fingerprint timing matrix:\n", times)

	print("Average generation time:\n", np.mean(gen_times))
	print("Average fingerprint time:\n", np.mean(times))

	matches = np.diagonal(results)
	stability_rate = np.mean(matches) / test_count
	print("Stability rate:\n", stability_rate)

	collisions = np.matrix(results) - np.diag(matches)
	collision_rate = np.mean(collisions) / test_count
	print("Collision rate:\n", collision_rate)

	### Alternate stability test of just the fingerprints with their corresponding machines
	# results, times = run_threaded_stability_test()
	# print("Fingerprint results matrix:\n", results)
	# print("Fingerprint timing matrix:\n", times)

	# print("Average fingerprint time:\n", np.mean(times))
	# stability_rate = np.mean(results) / test_count
	# print("Stability rate:\n", stability_rate)

if __name__ == '__main__':
	main()


