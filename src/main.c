#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>

#define errprintf(...) fprintf(stderr, __VA_ARGS__)
#define stdprintf(...) fprintf(stdout, __VA_ARGS__)
#define MAX_LEN_INPUT 100

struct command_line_arg{
    char* from;
    char* to;
    bool have_other_arg;
    bool have_arg;
    int count_from_arg;
    int count_to_arg;
};

struct array_sizes{
    int err_size;
    int stdout_size;
    int not_sorted_array_size;
};

struct borders{
    int from;
    int to;
    bool have_from;
    bool have_to;
};

struct validated_input{
    struct borders borders;
    int exit_code;
};

void bubble_sort(int *array, int array_size);
int calculate_changed_pos(const int *sorted_array, const int *not_sorted_array,int size){
    int cnt = 0;
    for(int i = 0; i < size; i++){
        if(sorted_array[i] != not_sorted_array[i]){
            cnt++;
        }
    }
    return cnt;
}


bool is_border_correct(char *border){
    return !(border == NULL) ;

}


struct command_line_arg parse_command_line_input(int argc, char *argv[]){
    struct command_line_arg arguments = {NULL,NULL,false,false,0,0};
    static struct option long_opt[] = {
            {"from", 1, 0, 'f'},
            {"to", 1, 0, 't'},
            {0,0,0,0},
    };
    int long_index;
    int opt = getopt_long( argc, argv, "f:t:", long_opt, &long_index );
    while(opt != -1){
        switch (opt) {
            case 'f':
                arguments.from =  optarg;
                arguments.have_arg = true;
                arguments.count_from_arg++;
                break;
            case 't':
                arguments.to =  optarg;
                arguments.have_arg = true;
                arguments.count_to_arg++;
                break;
            case '?':
                arguments.have_other_arg = true;
                break;
            default:
                break;
        }
        opt = getopt_long( argc, argv, "f:t:", long_opt, &long_index );
    }
    return arguments;

}

struct validated_input validate_command_line_input(struct command_line_arg arguments){
    struct borders borders = {0,0, false,false};
    struct validated_input input = {borders, 0};

    if(!arguments.have_arg){
        input.exit_code = -1;
    }
    if(arguments.have_other_arg){
        input.exit_code = -2;
    }
    if(arguments.count_to_arg > 1 || arguments.count_from_arg > 1){
        input.exit_code = -3;
    }
    if(arguments.count_to_arg == 1){
        input.borders.have_to = true;
    }
    if(arguments.count_from_arg == 1){
        input.borders.have_from = true;
    }
    if(arguments.count_from_arg == 0 && arguments.count_to_arg == 0){
        input.exit_code = -4;
    }

    if((is_border_correct(arguments.from) && arguments.count_from_arg) && !is_border_correct(arguments.to)){
        input.borders.from = strtol(arguments.from,&arguments.from,10);
    }
    else if(!is_border_correct(arguments.from) && (is_border_correct(arguments.to) && arguments.count_to_arg)){
        input.borders.to = strtol(arguments.to, &arguments.to, 10);
    }
    else{
        input.borders.from = strtol(arguments.from, &arguments.from,10);
        input.borders.to = strtol(arguments.to, &arguments.from,10);
    }

    return input;
}

struct validated_input command_line_input(int argc, char *argv[]){
    struct command_line_arg arguments = parse_command_line_input(argc, argv);
    return validate_command_line_input(arguments);
}

struct array_sizes std_input(struct borders borders, int *std_array,int *err_array, int *sorted_array,int *not_sorted_array){
    int num;
    int std_array_ind = 0, err_array_ind = 0,not_sorted_array_ind = 0;
    while(scanf("%d", &num)){
        if(borders.have_from && num <= borders.from){
            std_array[std_array_ind] = num;
            std_array_ind++;
        }
        else if(borders.have_to && num >= borders.to){
            err_array[err_array_ind] = num;
            err_array_ind++;
        }

        else {
            sorted_array[not_sorted_array_ind] = num;
            not_sorted_array[not_sorted_array_ind] = num;
            not_sorted_array_ind++;
        }
    }
    struct array_sizes sizes = {err_array_ind,std_array_ind,not_sorted_array_ind};
    return sizes;
}

void std_output(int *std_array, int size){
    for(int i = 0; i < size; i++){
        stdprintf("%d ", std_array[i]);
    }
}

void err_output(int *err_array, int size){
    for(int i = 0; i < size; i++){
        errprintf("%d ", err_array[i]);
    }
}

int main(int argc, char *argv[]) {
    struct validated_input input = command_line_input(argc, argv);
    if (input.exit_code < 0){
        return input.exit_code;
    }
    struct borders borders = input.borders;
    int not_sorted_array[MAX_LEN_INPUT];
    int sorted_array[MAX_LEN_INPUT];
    int std_array[MAX_LEN_INPUT];
    int err_array[MAX_LEN_INPUT];
    struct array_sizes sizes = std_input(borders, &std_array[MAX_LEN_INPUT], &err_array[MAX_LEN_INPUT],
            &sorted_array[MAX_LEN_INPUT],&not_sorted_array[MAX_LEN_INPUT]);
    std_output(&std_array[MAX_LEN_INPUT], sizes.stdout_size);
    err_output(&err_array[MAX_LEN_INPUT], sizes.err_size);
    bubble_sort( &sorted_array[MAX_LEN_INPUT], sizes.not_sorted_array_size);
    return calculate_changed_pos(&sorted_array[MAX_LEN_INPUT],&not_sorted_array[MAX_LEN_INPUT],
                                 sizes.not_sorted_array_size);
}
