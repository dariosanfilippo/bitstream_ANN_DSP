#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include "signals.c"
#include "osc.c"
#include "dsm.c"
#include "bitmath.c"
#include "data.c"
#include "kann.c"
#include "kautodiff.c"

int main(void) {

    kad_node_t* t;
    kann_t* ann;
    size_t inputs = 4096;
    size_t outputs = 1;
    size_t num_layers = 1;
    size_t neurons = 16;
    size_t SR = 192000;

    /* Create the neural network */
    t = kann_layer_input(inputs);
    for (size_t i = 0; i < num_layers; i++) {
        t = kann_layer_dense(t, neurons);
        t = kad_relu(t);
    }
    t = kann_layer_cost(t, outputs, KANN_C_MSE);
    ann = kann_new(t, 0);

    /* Create training data set */
    size_t setsize = 8192;
    Sig* x = malloc(sizeof(Sig));
    Sig* y = malloc(sizeof(Sig));
    sig_alloc(x, setsize, inputs, SR);
    sig_alloc(y, setsize, outputs, SR);
    freq_est_data(x, y);

    /* Train the net. */
    float lr = .001, frac_val = .1;
    size_t mini_size = 64;
    size_t max_epoch = 1000;
    size_t max_drop_streak = 20;
    kann_train_fnn1(ann, lr, mini_size, max_epoch, max_drop_streak, frac_val,
        setsize, x->vec_space, y->vec_space);
    sig_free(x);
    sig_free(y);

    /* Test the net. */
    size_t testsize = 10;
    x = malloc(sizeof(Sig));
    y = malloc(sizeof(Sig));
    sig_alloc(x, testsize, inputs, SR);
    sig_alloc(y, testsize, outputs, SR);
    freq_est_data(x, y);
    const float* output;
    for (size_t j = 0; j < testsize; j++) {
        output = kann_apply1(ann, x->vec_space[j]);
        printf("target: %.10f, ANN: %.10f, err: %.10f\n", 
            y->vec_space[j][0], *output, y->vec_space[j][0] / *output);
    }

    sig_free(x);
    sig_free(y);

    return EXIT_SUCCESS;
}