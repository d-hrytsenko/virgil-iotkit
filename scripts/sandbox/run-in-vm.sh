#!/bin/bash

VM_NAME="sandbox_f29"
START_SCRIPT="/vagrant/run.sh"

trap ctrlc_int INT

ctrlc_int() {
    echo "####### Found CTRL + C... Exiting"
    halt_vm
    exit 1
}

print_info_header() {
  echo "###########################################"
  echo "$1"
  echo "###########################################"
}

run_vm() {
    print_info_header "Starting VM using Vagrant"
    vagrant up ${VM_NAME}
}

run_sandbox() {
    print_info_header "Running IoT Sandbox inside VM"
    vagrant ssh ${VM_NAME} -c "${START_SCRIPT} vagrant"
}

halt_vm() {
    print_info_header "Halting VM"
    vagrant halt ${VM_NAME}
}


halt_vm
run_vm
run_sandbox
halt_vm
