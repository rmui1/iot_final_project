import requests
import json
import pprint

def get_recipes(food_counts_list, recommended_calories):
    food_list = [o['food'] for o in food_counts_list if o['count'] > 0]
    food_list = [f.replace(' ', '%20') for f in food_list]
    
    # get recipes that use available ingredents
    recipes_with_ingredients = requests.get('https://api.spoonacular.com/recipes/findByIngredients?ingredients={}&number=5&apiKey=ab9a3b0764c9481da5955493cfe866c5'.format(',+'.join(food_list))).text
    recipes_with_ingredients = json.loads(recipes_with_ingredients)

    # get nutrional info for above recipes
    recipes_with_nutrition = requests.get('https://api.spoonacular.com/recipes/informationBulk?ids={}&includeNutrition=true&apiKey=ab9a3b0764c9481da5955493cfe866c5'.format(','.join([str(r['id']) for r in recipes_with_ingredients]))).text
    recipes_with_nutrition = json.loads(recipes_with_nutrition)
    
    # for testing purposes
    # ingredient_query_file = open('data/ingredient_query.json')
    # recipes_with_ingredients = json.load(ingredient_query_file)
    # ingredient_query_file.close()

    # recipes_with_nutrition_file = open('data/bulk_info.json')
    # recipes_with_nutrition = json.load(recipes_with_nutrition_file)
    # recipes_with_nutrition_file.close()

    recipes_with_scores = []
    for i in range(len(recipes_with_ingredients)):
        r_i = recipes_with_ingredients[i]
        r_n = recipes_with_nutrition[i]

        score = 0
        used_ingredients = ','.join([i['name'] for i in r_i['usedIngredients']]).lower()

        # using more of available ingredients increases recipe score
        for f in food_counts_list:
            for w in f['food'].split():
                if w in used_ingredients:
                    score += f['count']
                    break
        
        calories = None
        fat = None
        protein = None
        carbs = None

        for n in r_n['nutrition']['nutrients']:
            if n['name'] == 'Calories':
                calories = n['amount']
            elif n['name'] == 'Fat':
                fat = n['amount']
            elif n['name'] == 'Protein':
                protein = n['amount']
            elif n['name'] == 'Carbohydrates':
                carbs = n['amount']
            
            if calories != None and fat != None and protein != None and carbs != None:
                break

        # fitting within recommended nutritional values increases recipe score
        meal_calories = recommended_calories / 3.0
        if calories < meal_calories:
            score += 1
        if carbs > 0.45 * meal_calories and carbs < 0.65 * meal_calories:
            score += 1
        if fat < 0.35 * meal_calories:
            score += 1
        if protein > 0.1 * meal_calories and protein < 0.35 * meal_calories:
            score += 1

        recipe = {
            'title': r_i['title'],
            'score': score,
            'calories': calories,
            'fat': fat,
            'protein': protein,
            'carbs': carbs,
            'used_ingredients': used_ingredients,
            'link': r_n['sourceUrl']
        }

        recipes_with_scores.append(recipe)

    # return recipes from highest to lowest score
    recipes_with_scores = sorted(recipes_with_scores, key=lambda r: r['score'], reverse=True)
        
    return recipes_with_scores

# pprint.pprint(get_recipes([{'food': 'sweet corn', 'count': 2}, {'food': 'sardine', 'count': 1}]))