#pragma once 

#include <opencv2/core.hpp>
#include <vector>	
#include <algorithm>
#include <array>

//steps used came from this video: https://www.youtube.com/watch?v=rrfFTdO2Z7I
namespace
{
	struct Point
	{
		int column, row;
		void Set(int column, int row)
		{
			this->column = column;
			this->row = row;
		}
	};
}

//Has a time complexity of fcking O(n^3). That's the worst case sceneario
class KuhnMunkres
{
public:
	KuhnMunkres() : size_n()
	{}
	/// \brief Solves the assignment problem for given dissimilarity matrix.
	/// It returns a vector that where each element is a column index for
	/// corresponding row (e.g. result[0] stores optimal column index for very
	/// first row in the dissimilarity matrix).
	/// \param dissimilarity_matrix CV_32F dissimilarity matrix.
	/// \return Optimal column index for each row. -1 means that there is no
	/// column for row.
	std::vector<size_t> Solve(const cv::Mat& dissimilarity);

private:
	typedef std::vector<Point> Position;

	size_t size_n;
	static constexpr int KStar = 1;  //mark points that has a value equal to zero
	static constexpr int KPrime = 2; //mark points that has a value less than zero
	
	cv::Mat m_dm;
	cv::Mat m_marked;
	cv::Point m_Point;

	std::vector<int> m_is_row_visited;
	std::vector<int> m_is_colmn_visited;
	std::vector<int> skip_colmns; // for row scanning only
	std::vector<int> skip_row; // Column scanning only

	Position ZeroPos; //main container where position of zeros is placed
	Position Zeros_at_rows;
	Position Zeros_at_cols;
	void ExecutePhases();

#pragma region phase_one
	void RowReduction();
	void ColmnReduction();
#pragma endregion

#pragma region phase_two
	//returns copy of a vector containg the position of zeros on each column
	void RowScanning();
	void ColScanning();
	bool IsOptimalFound();
#pragma endregion
};
