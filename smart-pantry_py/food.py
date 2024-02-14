from scipy.optimize import milp, LinearConstraint
import numpy as np

def get_available_food_list(food_info, total_weight):
    weight_tolerance = 2

    c = np.zeros(len(food_info) + 1)
    c[-1] = 1

    # sets up linear equations and upper/lower bounds
    equations = []
    equations.append([o['weight'] for o in food_info] + [1])
    for i in range(len(food_info)):
        equations.append([1 if j == i else 0 for j in range(len(food_info) + 1)])
    A = np.array(equations)

    b_u = np.concatenate([np.array([total_weight]), np.full(len(food_info), np.inf)])
    b_l = np.concatenate([np.array([total_weight]), np.zeros(len(food_info))])

    # version 1 of optimization problem: counts for all items must be positive integer
    integrality = np.concatenate([np.ones(len(food_info)), np.zeros(1)])

    constraints = LinearConstraint(A, b_l, b_u)
    res = milp(c=c, constraints=constraints, integrality=integrality)

    # perform version 2 if could not find solution/good enough solution
    need_recalculation = False
    try:
        need_recalculation = res.x[-1] > weight_tolerance
    except:
        try:
            need_recalculation = res.x == None
        except:
            need_recalculation = True

    if need_recalculation:
        # version 2 of optimization problem: item counts can be rational numbers to account for partially used items
        integrality = np.zeros(len(food_info) + 1)
        res = milp(c=c, constraints=constraints, integrality=integrality)

    food_list = [{'food': food_info[i]['food'], 'count': round(res.x[i], 2)} for i in range(len(food_info))]
    return food_list

# print(get_available_food_list([{'food': 'plum', 'weight': 7.05, 'count': 1}, {'food': 'sweet corn', 'weight': 15.25, 'count': 1}], 38))