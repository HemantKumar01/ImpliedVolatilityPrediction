#include <bits/stdc++.h>
using namespace std;

#define int long long
#define double long double

vector<vector<double>> input;
vector<int> rowNanCnt;
vector<vector<bool>> isNaN;
int filledByCallPut = 0;
int totalNanCnt = 0;

double THRESHOLD = 0.02; // RMSE threshold for considering two rows similar
int MAX_ITER = 25;
int MAX_PARITY_ITER = 0; // max iterations for using call put parity
int KNN_K = 5;           // to take avg

inline string trim(string &str)
{
  str.erase(str.find_last_not_of(' ') + 1); // suffixing spaces
  str.erase(0, str.find_first_not_of(' ')); // prefixing spaces
  return str;
}

void readCSV(string &inputFile)
{

  fstream fin;
  fin.open(inputFile, ios::in);
  int iter = 0;
  while (!fin.eof())
  {
    string line, word, temp;
    vector<string> row;
    vector<double> rowDouble;
    vector<bool> rowIsNaN(52, false);
    iter++;

    // read an entire row and
    // store it in a string variable 'line'
    getline(fin, line);

    // used for breaking words
    stringstream s(line);
    // read every column data of a row and
    // store it in a string variable, 'word'
    while (getline(s, word, ','))
    {
      // add all the column data
      // of a row to a vector
      word = trim(word);
      row.push_back(word);
    }
    // if last col is nan (empty), then it won't be recognized by default
    if (row.size() == 51)
    {
      row.push_back("");
    }
    if (row.size() == 0)
    {
      break;
    }
    if (row.size() != 52)
    {

      throw runtime_error("Invalid number of columns in the CSV file. Expected 52 columns, found " + to_string(row.size()) + " in row" + to_string(iter + 1));
    }
    int rowNanCntLocal = 0;
    for (int i = 0; i < (int)row.size(); i++)
    {
      if ((int)row[i].size() == 0LL)
      {
        rowDouble.push_back(-1.0);
        rowIsNaN[i] = true;
        totalNanCnt++;
        rowNanCntLocal++;
      }
      else
      {
        rowDouble.push_back(stold(row[i]));
      }
    }
    isNaN.push_back(rowIsNaN);
    rowNanCnt.push_back(rowNanCntLocal);
    input.push_back(rowDouble);
  }
  fin.close();
  cout << "Total rows read: " << input.size() << endl;
  cout << "Total Columns: " << input[0].size() << endl;
  cout << "Total NaN Cnt: " << totalNanCnt << endl;
}
void saveCSV(string &outputFile)
{
  cout << "Total NaN filled by call put parity: " << filledByCallPut << endl;
  cout << "Total NaN Cnt after filling: " << totalNanCnt << endl;
  fstream fout;
  fout.open(outputFile, ios::out);

  for (int i = 0; i < (int)input.size(); i++)
  {
    for (int j = 0; j < (int)input[i].size(); j++)
    {

      if (input[i][j] < 0)
      {
        fout << "";
      }
      else
      {
        fout << fixed << setprecision(6) << input[i][j];
      }
      if (j < (int)input[i].size() - 1)
      {
        fout << ",";
      }
    }
    fout << endl;
  }
  fout.close();
  cout << "Output saved to: " << outputFile << endl;
}

double getRMSE(int row1, int row2)
{
  double rmse = 0.0;
  int cnt = 0;
  for (int i = 0; i < (int)input[row1].size(); i++)
  {
    if (isNaN[row1][i] || isNaN[row2][i])
    {
      continue; // skip NaN values
    }
    rmse += (input[row1][i] - input[row2][i]) * (input[row1][i] - input[row2][i]);
    cnt++;
  }
  if (cnt < 4)
  {
    return 1e18;
  }
  return sqrt(rmse / cnt);
}
double getAvgDiff(int row1, int row2)
{
  //! note row2 - row1 (ORDER MATTERS)
  double avgDiff = 0.0;
  int cnt = 0;
  for (int i = 0; i < (int)input[row1].size(); i++)
  {
    if (isNaN[row1][i] || isNaN[row2][i])
    {
      continue; // skip NaN values
    }
    avgDiff += (input[row2][i] - input[row1][i]);
    cnt++;
  }
  if (cnt == 0)
  {
    return 1e18;
  }
  return avgDiff / cnt;
}

signed main(int32_t argc, char *argv[])
{
  if (argc != 3)
  {
    cout << "Usage: " << argv[0] << " <input_csv_file> <output_csv_file>" << endl;
    return 1;
  }
  string inputFile = argv[1];
  string outputFile = argv[2];

  readCSV(inputFile);
  int n = input.size();
  for (int i = 0; i < n; i++)
  {
    if (i % 1000 == 0)
    {
      cout << "Processing row " << i + 1 << " of " << n << endl;
    }
    vector<pair<double, int>> rmseList;
    for (int j = 0; j < n; j++)
    {
      if (i == j)
      {
        continue;
      }
      double rmse = getRMSE(i, j);
      rmseList.push_back({rmse, j});
    }
    sort(rmseList.begin(), rmseList.end());
    // for (int k = 0; k < (int)rmseList.size(); k++)
    // {
    //   if (rmseList[k].first > THRESHOLD || rowNanCnt[i] == 0 || k >= MAX_PARITY_ITER)
    //   {
    //     break; // stop if the RMSE is too high or no NaN values in the row
    //   }
    //   int callputsize = (int)input[i].size() / 2;
    //   for (int j = 0; j < callputsize; j++)
    //   {
    //     if (input[i][j] < 0 || input[i][j + callputsize] < 0)
    //     {
    //       if (!isNaN[rmseList[k].second][j + callputsize] && !isNaN[rmseList[k].second][j])
    //       {
    //         if (input[i][j + callputsize] > 0)
    //         {
    //           // if the call put parity is satisfied, fill the NaN value
    //           input[i][j] = input[rmseList[k].second][j] - input[rmseList[k].second][j + callputsize] + input[i][j + callputsize];
    //           isNaN[i][j] = false; // mark it as filled
    //           filledByCallPut++;
    //           totalNanCnt--;
    //           rowNanCnt[i]--;
    //         }
    //         else if (input[i][j] > 0)
    //         {
    //           input[i][j + callputsize] = input[rmseList[k].second][j + callputsize] - input[rmseList[k].second][j] + input[i][j];
    //           isNaN[i][j] = false; // mark it as filled
    //           filledByCallPut++;
    //           totalNanCnt--;
    //           rowNanCnt[i]--;
    //         }
    //       }
    //     }
    //   }
    // }

    for (int k = 0; k < (int)rmseList.size(); k++)
    {
      if (rmseList[k].first > THRESHOLD || rowNanCnt[i] == 0 || k >= MAX_ITER)
      {
        break; // stop if the RMSE is too high or no NaN values in the row
      }

      for (int j = 0; j < (int)input[i].size(); j++)
      {
        // if (input[i][j] < 0 && input[rmseList[k].second][j] > 0)
        if (input[i][j] < 0 && !isNaN[rmseList[k].second][j])

        {
          // double avgDiff = getAvgDiff(i, rmseList[k].second);
          // if (abs(avgDiff) > THRESHOLD * 100)
          // {
          //   cout << "ALERT: AvgDiff is too high for row " << i << " and row " << rmseList[k].second << " rmse: " << rmseList[k].first << endl;
          //   avgDiff = 0.0; // if avgDiff is too high, set it to 0
          // }

          input[i][j] = input[rmseList[k].second][j]; // fill NaN with the first closest row
          isNaN[i][j] = false;                        // mark it as filled
          totalNanCnt--;
          rowNanCnt[i]--;
        }
      }
    }
  }
  saveCSV(outputFile);
  return 0;
}