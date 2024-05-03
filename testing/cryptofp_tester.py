import paramiko
import re
import sys
import threading
import time
from scp import SCPClient

aws_vms = [
	"ec2-3-149-27-128.us-east-2.compute.amazonaws.com",
	"ec2-3-135-203-55.us-east-2.compute.amazonaws.com"
]

test_count = 5

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
			client.exec_command("unzip LicenseLocking-main.zip")
			stdin, stdout, stderr = client.exec_command("cd LicenseLocking/cryptofp-cpp && ./build.sh && make")
			print(stdout.readlines())
			print(stderr.readlines())
			print("Built LicenseLocking package for machine " + str(i))
		else:
			print("LicenseLocking package was already initialized for machine " + str(i))

		client.close()


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


def download_fingerprints():
	for i in range(len(aws_vms)):
		client = create_ssh_client(aws_vms[i], "ubuntu", "server-1-keypair.pem")
		fp_path = "~/fp-" + str(i)
		scp_retrieve_file(client, fp_path)

		print("Downloaded fingerprint for machine " + str(i))

		client.close()


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



def run_test_loop():
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


#############################################
####		   Main Execution			 ####
#############################################
def main():
	initialize_clients()
	gen_times = generate_fingerprints()
	print(gen_times)
	time.sleep(1)
	download_fingerprints()
	upload_fingerprints()
	results, times = run_test_loop()
	print(results)
	print(times)

if __name__ == '__main__':
	main()


