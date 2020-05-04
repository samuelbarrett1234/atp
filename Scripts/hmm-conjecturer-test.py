from random import random
import numpy as np

def generate_expr(state, st_trans, st_obs, p):
    state = st_trans @ state  # state transition
    obs = st_obs @ state  # observation probabilities
    assert(obs.shape == (3,))
    r = random()
    if r < obs[0]:
        return "i(" + generate_expr(state, st_trans, st_obs, p) + ")"
    elif r < obs[0] + obs[1]:
        expr1 = generate_expr(state, st_trans, st_obs, p)
        expr2 = generate_expr(state, st_trans, st_obs, p)
        return "*(" + expr1 + ", " + expr2 + ")"
    else:
        i = 0
        while random() < p:
            i += 1
        return "x" + str(i)


N = 3
st_trans = np.array([[0.3, 0.2, 0.5], [0.2, 0.2, 0.6], [0.3, 0.3, 0.4]])
st_obs = np.array([[0.5, 0.3, 0.2], [0.2, 0.5, 0.3], [0.15, 0.15, 0.7]])
p = 0.25
initial_state = np.array([0.45, 0.45, 0.1])
for i in range(10):
    print (generate_expr(np.copy(initial_state),
        st_trans, st_obs, p) + " = " +
        generate_expr(np.copy(initial_state),
        st_trans, st_obs, p))
