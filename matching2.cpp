#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <limits>
#include <algorithm>
#include <iomanip>
#include <unordered_map>
#include <chrono>
#include <bits/stdc++.h>

class CSVProcessor
{
private:
  std::vector<std::vector<double>> trainData;
  std::vector<std::vector<bool>> trainIsNaN;
  std::vector<std::vector<double>> testData;
  std::vector<std::vector<bool>> testIsNaN;

  std::vector<std::string> trainHeaders;
  std::vector<std::string> testHeaders;
  std::vector<int> commonColumnIndicesTest;  // Index mapping from test to common columns
  std::vector<int> commonColumnIndicesTrain; // Index mapping from train to common columns

  int trainRows, trainCols, testRows, testCols, commonCols;
  const double NaN = std::numeric_limits<double>::quiet_NaN();
  const double RMSE_THRESHOLD = 0.002;

public:
  std::vector<std::string> parseCSVLine(const std::string &line)
  {
    std::vector<std::string> result;
    size_t start = 0;
    size_t pos = 0;

    // Parse the line character by character to handle all comma cases
    while (pos <= line.length())
    {
      if (pos == line.length() || line[pos] == ',')
      {
        // Extract cell from start to pos
        std::string cell = (pos > start) ? line.substr(start, pos - start) : "";

        // Trim whitespace
        cell.erase(0, cell.find_first_not_of(" \t\r\n"));
        cell.erase(cell.find_last_not_of(" \t\r\n") + 1);

        result.push_back(cell);
        start = pos + 1;
      }
      pos++;
    }

    return result;
  }

  bool loadCSV(const std::string &filename, bool isTrain)
  {
    std::ifstream file(filename);
    if (!file.is_open())
    {
      std::cerr << "Error: Cannot open file " << filename << std::endl;
      return false;
    }

    std::string line;
    bool isFirstLine = true;

    auto &data = isTrain ? trainData : testData;
    auto &isNaN = isTrain ? trainIsNaN : testIsNaN;
    auto &headers = isTrain ? trainHeaders : testHeaders;

    data.clear();
    isNaN.clear();
    headers.clear();

    while (std::getline(file, line))
    {
      auto cells = parseCSVLine(line);

      if (isFirstLine)
      {
        // First line is header
        headers = cells;
        isFirstLine = false;
        continue;
      }

      std::vector<double> row;
      std::vector<bool> nanRow;

      for (const auto &cell : cells)
      {
        if (cell.empty() || cell == "nan" || cell == "NaN" || cell == "NULL" || cell == "NA")
        {
          row.push_back(NaN);
          nanRow.push_back(true);
        }
        else
        {
          try
          {
            double val = std::stod(cell);
            if (std::isnan(val))
            {
              row.push_back(NaN);
              nanRow.push_back(true);
            }
            else
            {
              row.push_back(val);
              nanRow.push_back(false);
            }
          }
          catch (const std::exception &)
          {
            row.push_back(NaN);
            nanRow.push_back(true);
          }
        }
      }

      if (!row.empty())
      {
        data.push_back(row);
        isNaN.push_back(nanRow);
      }
    }

    file.close();

    if (data.empty())
    {
      std::cerr << "Error: No data loaded from CSV " << filename << std::endl;
      return false;
    }

    if (isTrain)
    {
      trainRows = data.size();
      trainCols = data[0].size();
      std::cout << "Loaded train data: " << trainRows << " rows and " << trainCols << " columns" << std::endl;
    }
    else
    {
      testRows = data.size();
      testCols = data[0].size();
      std::cout << "Loaded test data: " << testRows << " rows and " << testCols << " columns" << std::endl;
    }

    return true;
  }

  void findCommonColumns()
  {
    commonColumnIndicesTest.clear();
    commonColumnIndicesTrain.clear();

    // Create mapping from column name to index for train data
    std::unordered_map<std::string, int> trainColMap;
    for (int i = 0; i < trainHeaders.size(); i++)
    {
      trainColMap[trainHeaders[i]] = i;
    }

    // Find common columns
    for (int i = 0; i < testHeaders.size(); i++)
    {
      auto it = trainColMap.find(testHeaders[i]);
      if (it != trainColMap.end())
      {
        commonColumnIndicesTest.push_back(i);
        commonColumnIndicesTrain.push_back(it->second);
      }
    }

    commonCols = commonColumnIndicesTest.size();

    std::cout << "Found " << commonCols << " common columns:" << std::endl;
    for (int i = 0; i < commonCols; i++)
    {
      std::cout << "  " << testHeaders[commonColumnIndicesTest[i]] << " (test col "
                << commonColumnIndicesTest[i] << " -> train col "
                << commonColumnIndicesTrain[i] << ")" << std::endl;
    }
  }

  double calculateRMSE(int testRow, int trainRow)
  {
    double sum = 0.0;
    int count = 0;

    for (int i = 0; i < commonCols; i++)
    {
      int testCol = commonColumnIndicesTest[i];
      int trainCol = commonColumnIndicesTrain[i];

      // Both values must be non-NaN for comparison
      if (!testIsNaN[testRow][testCol] && !trainIsNaN[trainRow][trainCol])
      {
        double diff = testData[testRow][testCol] - trainData[trainRow][trainCol];
        sum += diff * diff;
        count++;
      }
    }

    // Require at least 4 common non-NaN values for valid RMSE
    if (count < 3)
    {
      return std::numeric_limits<double>::infinity();
    }

    return std::sqrt(sum / count);
  }

  std::pair<int, int> findBestMatchingTrainRows(int testRow)
  {
    std::vector<std::pair<double, int>> rmseValues;

    for (int i = 0; i < trainRows; i++)
    {
      double rmse = calculateRMSE(testRow, i);
      if (rmse < std::numeric_limits<double>::infinity() && rmse <= RMSE_THRESHOLD)
      {
        rmseValues.push_back({rmse, i});
      }
    }

    if (rmseValues.size() < 2)
    {
      return {-1, -1}; // Not enough valid rows with RMSE <= threshold
    }

    // Sort by RMSE and pick top 2
    std::sort(rmseValues.begin(), rmseValues.end());

    return {rmseValues[0].second, rmseValues[1].second};
  }

  void fillNaNValues()
  {
    if (commonCols == 0)
    {
      std::cout << "No common columns found. Cannot fill NaN values." << std::endl;
      return;
    }

    std::cout << "Processing test rows for NaN filling..." << std::endl;
    std::cout << "RMSE threshold: " << RMSE_THRESHOLD << std::endl;

    int rowsProcessed = 0;
    int rowsWithNaN = 0;
    int rowsWithMatches = 0;
    int nansFilled = 0;

    for (int i = 0; i < testRows; i++)
    {
      if (i % 1000 == 0 && i > 0)
      {
        std::cout << "Processing row " << i << "/" << testRows << std::endl;
      }

      // Check if current test row has any NaN values in common columns
      bool hasNaN = false;
      for (int j = 0; j < commonCols; j++)
      {
        int testCol = commonColumnIndicesTest[j];
        if (testIsNaN[i][testCol])
        {
          hasNaN = true;
          break;
        }
      }

      if (!hasNaN)
      {
        rowsProcessed++;
        continue; // Skip if no NaN values in common columns
      }

      rowsWithNaN++;

      // Find best matching train rows
      auto bestRows = findBestMatchingTrainRows(i);

      if (bestRows.first == -1 || bestRows.second == -1)
      {
        rowsProcessed++;
        continue; // Skip if can't find suitable matches within threshold
      }

      rowsWithMatches++;

      // Fill NaN values from the two best matching train rows
      for (int j = 0; j < commonCols; j++)
      {
        int testCol = commonColumnIndicesTest[j];
        int trainCol = commonColumnIndicesTrain[j];

        if (testIsNaN[i][testCol])
        {
          // Try to fill from first best match
          if (!trainIsNaN[bestRows.first][trainCol])
          {
            testData[i][testCol] = trainData[bestRows.first][trainCol];
            testIsNaN[i][testCol] = false;
            nansFilled++;
          }
          // If still NaN, try second best match
          else if (!trainIsNaN[bestRows.second][trainCol])
          {
            testData[i][testCol] = trainData[bestRows.second][trainCol];
            testIsNaN[i][testCol] = false;
            nansFilled++;
          }
          // If both matches also have NaN, keep as NaN
        }
      }
      rowsProcessed++;
    }

    std::cout << "NaN filling completed!" << std::endl;
    std::cout << "Statistics:" << std::endl;
    std::cout << "  Rows processed: " << rowsProcessed << std::endl;
    std::cout << "  Rows with NaN values: " << rowsWithNaN << std::endl;
    std::cout << "  Rows with valid matches (RMSE <= " << RMSE_THRESHOLD << "): " << rowsWithMatches << std::endl;
    std::cout << "  NaN values filled: " << nansFilled << std::endl;
  }

  bool saveCSV(const std::string &filename)
  {
    std::ofstream file(filename);
    if (!file.is_open())
    {
      std::cerr << "Error: Cannot create output file " << filename << std::endl;
      return false;
    }

    file << std::fixed << std::setprecision(6);

    // Write header
    for (int j = 0; j < testHeaders.size(); j++)
    {
      if (j > 0)
        file << ",";
      file << testHeaders[j];
    }
    file << "\n";

    // Write data
    for (int i = 0; i < testRows; i++)
    {
      for (int j = 0; j < testCols; j++)
      {
        if (j > 0)
          file << ",";

        if (testIsNaN[i][j] || std::isnan(testData[i][j]))
        {
          file << "NaN";
        }
        else
        {
          file << testData[i][j];
        }
      }
      file << "\n";
    }

    file.close();
    std::cout << "Saved processed test data to " << filename << std::endl;
    return true;
  }

  void printStats()
  {
    int totalTestCells = testRows * testCols;
    int testNanCount = 0;

    for (int i = 0; i < testRows; i++)
    {
      for (int j = 0; j < testCols; j++)
      {
        if (testIsNaN[i][j] || std::isnan(testData[i][j]))
          testNanCount++;
      }
    }

    std::cout << "Final Statistics:" << std::endl;
    std::cout << "Test data - Total cells: " << totalTestCells << std::endl;
    std::cout << "Test data - NaN cells: " << testNanCount << " ("
              << (100.0 * testNanCount / totalTestCells) << "%)" << std::endl;
  }
};

int main(int argc, char *argv[])
{
  if (argc != 4)
  {
    std::cout << "Usage: " << argv[0] << " <train_csv_file> <test_csv_file> <output_csv_file>" << std::endl;
    std::cout << "Example: " << argv[0] << " train.csv test.csv filled_test.csv" << std::endl;
    std::cout << "\nNote: Both CSV files should have headers in the first row." << std::endl;
    std::cout << "      RMSE threshold is set to 0.002" << std::endl;
    return 1;
  }

  std::string trainFile = argv[1];
  std::string testFile = argv[2];
  std::string outputFile = argv[3];

  CSVProcessor processor;

  std::cout << "Loading train CSV file: " << trainFile << std::endl;
  if (!processor.loadCSV(trainFile, true))
  {
    return 1;
  }

  std::cout << "Loading test CSV file: " << testFile << std::endl;
  if (!processor.loadCSV(testFile, false))
  {
    return 1;
  }

  std::cout << "\nFinding common columns..." << std::endl;
  processor.findCommonColumns();

  processor.printStats();

  std::cout << "\nStarting NaN filling process..." << std::endl;
  auto start = std::chrono::high_resolution_clock::now();

  processor.fillNaNValues();

  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  std::cout << "Processing completed in " << duration.count() << " ms" << std::endl;

  processor.printStats();

  if (!processor.saveCSV(outputFile))
  {
    return 1;
  }

  std::cout << "Process completed successfully!" << std::endl;
  return 0;
}