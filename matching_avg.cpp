#include <bits/stdc++.h>
using namespace std;

#define int long long
#define double long double

vector<vector<double>> input;
vector<int> rowNanCnt;
vector<vector<bool>> isNaN;
int totalNanCnt = 0;

double THRESHOLD = 0.01; // RMSE threshold for considering two rows similar
int MAX_ITER = 15;

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
    if (input[row1][i] < 0 || input[row2][i] < 0)
    {
      continue; // skip NaN values
    }
    rmse += (input[row1][i] - input[row2][i]) * (input[row1][i] - input[row2][i]);
    cnt++;
  }
  if (cnt < 6)
  {
    return 1e18;
  }
  return sqrt(rmse / cnt);
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

    for (int j = 0; j < (int)input[i].size(); j++)
    {
      if (input[i][j] < 0)
      {
        double sum_col = 0;
        double cnt_col = 0;
        for (int k = 0; k < (int)rmseList.size(); k++)
        {
          if (rmseList[k].first > THRESHOLD || rowNanCnt[i] == 0 || k >= MAX_ITER)
          {
            break; // stop if the RMSE is too high or no NaN values in the row
          }
          // if (input[i][j] < 0 && input[rmseList[k].second][j] > 0)
          if (!isNaN[rmseList[k].second][j])
          {
            if (i == 1 && j == 4)
            {
              cout << rmseList[k].second << " " << rmseList[k].first << endl;
            }
            sum_col += input[rmseList[k].second][j];
            cnt_col++;
            // input[i][j] = input[rmseList[k].second][j]; // fill NaN with the first closest row
            // totalNanCnt--;
            // rowNanCnt[i]--;
          }
        }
        if (cnt_col > 0)
        {
          input[i][j] = sum_col / cnt_col; // fill NaN with the average of the closest rows
          totalNanCnt--;
          rowNanCnt[i]--;
        }
      }
    }
  }
  saveCSV(outputFile);
  return 0;
}