import pandas as pd
import re

# load dataset and filter for only food items
data = pd.read_csv('data/walmart.csv')
data = data.fillna('')
data = data[data['Category'].str.contains('Food')]
data = data[['Product Name']]

def get_object_weights(food_list):
    weights = {}
    for f in food_list:
        rows = data.copy()

        # get rows that contain food item with weight information
        rows = rows[rows['Product Name'].str.contains(f, case=False)]
        rows = rows[rows['Product Name'].str.contains(' oz', case=False)]
        
        # extract weight information from product name
        if len(rows) > 0:
            raw = rows.iloc[0]['Product Name']
            raw = re.findall(r'(\d+(\.\d+)?) [Oo]z', raw)
            raw = list(sum(raw, ()))
            raw = max(raw, key=len)
            weights[f] = float(raw)
    
    return weights
        
# print(get_object_weights(['sweet corn', 'pickled plum']))