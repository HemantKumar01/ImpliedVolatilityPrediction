# NKSR Hackathon approaches
https://www.kaggle.com/competitions/nk-iv-prediction/
i didn't include any data files here

- You should start by `explore.ipynb`
- what worked best is `matching.cpp` which finds the nearest neighbour (based on rmse) like knn and imputes it's non nan values into - current row. Since the IV values does not change much in a second, it is supposed to find a temporal order. You need to prepare the data into csv with only call put iv columns without any header for this code to work.
- then `cubic_spline.ipynb` for remaining ipynb (if during matching MAX_ITER is reached or rmse < THRESHOLD)
- finally applying savgol filter or pca smoothing made predictions better (bot in `final_day/pca_smoothing.ipynb`)
- `final_day/lol.csv` also works good, it uses xgb with iteratively improving the completely filled data (initially guessed through matching and cubic spline)

- I tried many other things like svi or pchip instead of cubic spline but they did not fit correctly in a many rows.
- I also tried using call put parity, training xgb on train data's each column to predict previous column, it works fine but not as good as cubic spline. 
