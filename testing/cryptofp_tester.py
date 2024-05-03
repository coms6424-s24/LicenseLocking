import paramiko
import re
import sys
import time
from scp import SCPClient

aws_vms = [
	"ec2-3-135-216-61.us-east-2.compute.amazonaws.com",
	"ec2-3-17-74-69.us-east-2.compute.amazonaws.com"
]

test_count = 10

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
			client.exec_command("unzip LicenseLocking-main.zip")
			stdin, stdout, stderr = client.exec_command("cd LicenseLocking/cryptofp-cpp && ./build.sh && make")
			print(stdout.readlines())
			print(stderr.readlines())
			print("Built LicenseLocking package for machine " + str(i))
		else:
			print("LicenseLocking package was already initialized for machine " + str(i))

		client.close()


def generate_fingerprints():
	for i in range(len(aws_vms)):
		client = create_ssh_client(aws_vms[i], "ubuntu", "server-1-keypair.pem")
		fp_path = "~/fp-" + str(i)
		command_string = "cd LicenseLocking/cryptofp-cpp && sudo ./select_timer.sh hpet && ./main " + fp_path;
		stdin, stdout, stderr = client.exec_command(command_string)

		print("Generated fingerprint for machine " + str(i))

		client.close()


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

	for client_num in range(len(aws_vms)):
		client = create_ssh_client(aws_vms[client_num], "ubuntu", "server-1-keypair.pem")

		for fp_num in range(len(aws_vms)):
			fp_path = "test_fps/fp-" + str(fp_num)
			test_param = "--test_count=" + str(test_count)
			command_string = "cd LicenseLocking/cryptofp-cpp && ./taskset_test.sh --fp=" + fp_path + " " + test_param + " --stress=0"
			stdin, stdout, stderr = client.exec_command(command_string)
			cmd_output = stdout.readline()

			m = re.match(r'cpu 0: (\d+)/(\d+)', cmd_output)
			fail_count = m.group(1)
			pass_count = test_count - int(fail_count)
			results[client_num][fp_num] = pass_count

			print("Testing fingerprint " + str(fp_num) + " on machine " + str(client_num) + " with results: " + str(pass_count) + "/" + str(test_count))

		client.close()

	return results


#############################################
####		   Main Execution			 ####
#############################################
def main():
	initialize_clients()
	generate_fingerprints()
	time.sleep(1)
	download_fingerprints()
	upload_fingerprints()
	results = run_test_loop()
	print(results)

if __name__ == '__main__':
	main()


