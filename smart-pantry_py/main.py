from flask import Flask, flash, redirect, render_template, request, url_for
import pprint
import json
import time

from food import get_available_food_list
from recipe import get_recipes
from aws import get_pantry_info
from cv import save_image_and_get_visible_food
from weight import get_object_weights

app = Flask(__name__)

@app.route('/')
def index():
    return render_template(
        'form.html', 
        dropdowns=[
            {'name': 'Age_Range', 'options': ['18-29', '30-49', '50-64', '65-74', '75+']},
            {'name': 'Demo_Option', 'options': [0, 1, 2]},
        ], 
        textboxes=['Weight(kg)', 'Food_Weight(oz)']
    )

@app.route("/calculations" , methods=['GET', 'POST'])
def calculations():
    age_range = request.form.get('Age_Range')
    try:
        weight = float(request.form.get('Weight(kg)'))
    except:
        return 'Invalid weight'

    calories = weight
    if age_range == '18-29':
        calories *= 33.2
    elif age_range == '30-49':
        calories *= 32.9
    elif age_range == '50-64':
        calories *= 31.1
    elif age_range == '65-74':
        calories *= 30.0
    else:
        calories *= 29.0
    calories = round(calories)

    food_info_dict= {}

    # option 0 gets information from sensors, options 1 and 2 from pretaken images and user input
    demo_option = int(request.form.get('Demo_Option'))
    if demo_option == 0:
        image_path = './static/captured_image.jpg'
        received_food_weight = food_weight = get_pantry_info()
    else:
        image_path = './static/pretaken_image_{}.jpg'.format(demo_option)
        received_food_weight = -1
        try:
            food_weight = float(request.form.get('Food_Weight(oz)'))
        except:
            return 'Invalid food weight'

    food_list = save_image_and_get_visible_food(image_path)
    for f in food_list:
        f = f.strip()
        if f in food_info_dict:
            food_info_dict[f] += 1
        else:
            food_info_dict[f] = 1

    weights = get_object_weights(list(food_info_dict.keys()))

    food_info = []
    for f in food_info_dict:
        food_info.append({'food': f, 'weight': weights[f], 'count': food_info_dict[f]})
        
    inferred = get_available_food_list(food_info, food_weight)
    recipes = get_recipes(inferred, calories)

    return render_template(
        'recipes.html', 
        recipes=recipes,
        food_weight=food_weight,
        received_food_weight=received_food_weight,
        visible=food_info_dict,
        inferred=inferred,
        calories=calories,
        image_path=image_path
    )

if __name__ == '__main__':
    app.run(debug=True)
