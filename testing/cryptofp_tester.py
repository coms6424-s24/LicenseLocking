import paramiko
import re
import sys
import time
from scp import SCPClient

aws_vms = [
	"ec2-52-14-168-126.us-east-2.compute.amazonaws.com"
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
		stdin, stdout, stderr = client.exec_command("ls LicenseLocking-main/cryptofp-cpp/main")
		ls_result = stderr.readlines()

		if (len(ls_result) > 0):
			scp_send_file(client, "LicenseLocking-main.zip")
			client.exec_command("unzip LicenseLocking-main.zip")
			stdin, stdout, stderr = client.exec_command("cd LicenseLocking-main/cryptofp-cpp && ./build.sh && make")
			print("Built LicenseLocking package for machine " + str(i))
		else:
			print("LicenseLocking package was already initialized for machine " + str(i))

		client.close()


def generate_fingerprints():
	return


def download_fingerprints():
	return


def upload_fingerprints():
	return


def run_test_loop():
	results = [[0 for x in range(len(aws_vms))] for y in range(len(aws_vms))]

	for client_num in range(len(aws_vms)):
		client = create_ssh_client(aws_vms[client_num], "ubuntu", "server-1-keypair.pem")

		for fp_num in range(len(aws_vms)):
			fp_path = "~/test_fps/fp-" + str(fp_num)
			test_param = "--test_count=" + str(test_count)
			command_string = "cd LicenseLocking-main/cryptofp-cpp && ./taskset_test.sh --fp=" + fp_path + " " + test_param + " --stress=0"
			stdin, stdout, stderr = client.exec_command(command_string)
			cmd_output = stdout.readline()

			m = re.match(r'cpu 0: (\d+)/(\d+)', cmd_output)
			fail_count = m.group(1)
			results[i][j] = test_count - fail_count

		client.close()

	return results


#############################################
####		   Main Execution			 ####
#############################################
def main():
	# initialize_clients()
	# generate_fingerprints()
	# download_fingerprints()
	# upload_fingerprints()
	#results = run_test_loop()


	client = create_ssh_client("ec2-52-14-168-126.us-east-2.compute.amazonaws.com", "ubuntu", "server-1-keypair.pem")
	#stdin, stdout, stderr = client.exec_command("cd LicenseLocking-main/cryptofp-cpp && sudo ./select_timer.sh hpet && ./main fingerprints/aws-fp-dec1 -cmp")
	stdin, stdout, stderr = client.exec_command("cd LicenseLocking-main-old/cryptofp-cpp && ./taskset_test.sh --fp=fingerprints/aws-fp-dec1 --test_count=1 --stress=0")
	res = stdout.readline()
	m = re.match(r'cpu 0: (\d+)/(\d+)', res)

	print(m.group(1))
	client.close()

	


if __name__ == '__main__':
	main()





def invoke_ssh(client):
	# channel = client.invoke_shell()
	# stdin = channel.makefile('wb')
	# stdout = channel.makefile('rb')

	# stdin.write('''
	# cd LicenseLocking-main/cryptofp-cpp
	# ./main fingerprints/aws-fp-dec1 -cmp
	# exit
	# ''')
	# print(stdout.readlines())

	# stdout.close()
	# stdin.close()
	# client.close()
	return



