/*Copyright (C) 2014 Wayne Mogg All rights reserved.

This file may be used either under the terms of:

1. The GNU General Public License version 3 or higher, as published by
the Free Software Foundation, or

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef statcalc_h
#define statcalc_h

/*+
________________________________________________________________________

 Author:        Wayne Mogg
 Date:          January 2014
 ________________________________________________________________________

-*/ 
namespace wmStats
{

template <class T>
class StatCalc
{
  public:
    StatCalc() { clear(); }
    ~StatCalc() {}

    inline void		clear();
    inline StatCalc<T>& addValue(T data);
    inline T		median();
    inline double	average() const;
    inline double	variance();
    inline T		min() const;
    inline T		max() const;
    inline T		iqr();

protected:

    TypeSet<T>	vals_;
    int		minidx_;
    int	 	maxidx_;
    T		maxval_;
    T		minval_;
    T		mean_;
    T		m2_;
}; 

template <class T> inline
void StatCalc<T>::clear()
{
  mean_ = m2_ = 0;
  minval_ = maxval_ = mUdf(T);
  vals_.erase();
}

template <class T>
inline double StatCalc<T>::average() const
{
  const int sz = vals_.size();
  if (sz < 1 ) return mUdf(T);
  return mean_;
}

template <class T>
inline double StatCalc<T>::variance()
{
  const int sz = vals_.size();
  if ( sz < 2 ) return 0;
  return ((double)m2_)/(sz-1);
}

template <class T>
inline T StatCalc<T>::min( ) const
{
    return minval_;
}


template <class T>
inline T StatCalc<T>::max() const
{
    return maxval_;
}

template <class T>
inline T StatCalc<T>::iqr() 
{
  const int sz = vals_.size();
  T Q3, Q1;
  std::vector<T>& vec = vals_.vec();

  if ( sz < 2 )
      return sz < 1 ? mUdf(T) : 0;

  const int idx25 = sz / 4;
  const int idx75 = 3 * sz / 4;

  std::nth_element( vec.begin(), vec.begin()+idx75, vec.end());
  if (sz%4 ==0) 
    Q1 = (vec[idx25]+vec[idx25-1]) / 2;
  else
    Q1 = vec[idx25];
  if ((3*sz)%4==0)
    Q3 = (vec[idx75]+vec[idx75-1]) / 2;
  else
    Q3 = vec[idx75];
  return Q3-Q1;
}


template <class T>
inline T StatCalc<T>::median()
{
  const int sz = vals_.size();
  std::vector<T>& vec = vals_.vec();

  if ( sz < 2 )
      return sz < 1 ? mUdf(T) : vec[0];

  const int mididx = sz / 2;

  std::nth_element( vec.begin(), vec.begin()+mididx, vec.end());
  
  if ( sz%2 == 0 )
    return (vec[mididx] + vec[mididx-1]) / 2;
  else
    return vec[mididx];
}

template <class T> inline
StatCalc<T>& StatCalc<T>::addValue( T val)
{
    if ( mIsUdf(val) )
	return *this;

    const int sz = vals_.size();
    if (sz > 0)
    {
      if ( val < minval_ ) { minval_ = val; }
      if ( val > maxval_ ) { maxval_ = val; }
    }
    else
    {
      minval_ = maxval_ = val;
    }
    T delta = val - mean_;
    mean_ = mean_ + delta/(sz+1);
    m2_ = m2_ + delta * ( val - mean_);

    vals_ += val;

    return *this;
}

}
#endif