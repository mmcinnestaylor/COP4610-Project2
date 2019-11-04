SHELL := /bin/bash
ELEVATOR_MODULE = /usr/src/test_kernel/part3
.PHONY: compile insert remove start issue stop watch_proc clean
.PHONY1: compile insert 


compile: driver.c producer.c consumer.c wrappers.h
	gcc -o producer.x producer.c
	gcc -o consumer.x consumer.c
	gcc -o driver.x driver.c
insert:
	make -C $(ELEVATOR_MODULE) && sudo insmod $(ELEVATOR_MODULE)/elevator.ko
remove:
	sudo rmmod elevator

driver: compile
	./driver.x	
start: compile
	./consumer.x --start
issue: compile
	./producer.x
issues: compile
	for run in {1..50}; do \
		./producer.x; \
		sleep 1; \
	done
stop: compile
	./consumer.x --stop

watch_proc:
	while [ 1 ]; do \
		clear; clear; \
		cat /proc/elevator; \
		sleep 1; \
	done

clean:
	rm *.x
