all: dpp_DL dpp_DLF_semaphore dpp_DLF_mutex dpp_DLF_oddeven

dpp_DL: dpp_DL.c
	gcc dpp_DL.c -o dpp_DL -lpthread
	@echo run as ./dpp_DL -n num_of_philosophers

dpp_DLF_oddeven: dpp_DLF_oddeven.c
	gcc dpp_DLF_oddeven.c -o dpp_DLF_oddeven -lpthread
	@echo run as ./dpp_DLF_oddeven -n num_of_philosophers

dpp_DLF_mutex: dpp_DLF_mutex.c
	gcc dpp_DLF_mutex.c -o dpp_DLF_mutex -lpthread
	@echo run as ./dpp_DLF_mutex -n num_of_philosophers

dpp_DLF_semaphore: dpp_DLF_semaphore.c
	gcc dpp_DLF_semaphore.c -o dpp_DLF_semaphore -lpthread -lm
	@echo run as ./dpp_DLF_semaphore -n num_of_philosophers
