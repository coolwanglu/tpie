// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, The TPIE development team
// 
// This file is part of TPIE.
// 
// TPIE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
// 
// TPIE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with TPIE.  If not, see <http://www.gnu.org/licenses/>

#ifndef _TPIE_PROGRESS_INDICATOR_TERMINAL_H
#define _TPIE_PROGRESS_INDICATOR_TERMINAL_H

#include <tpie/portability.h>

#include <tpie/progress_indicator_base.h>

namespace tpie 
{

///////////////////////////////////////////////////////////////////
///
///  A class that indicates the progress by a simple counter that
///  is printed to the terminal.
///
///////////////////////////////////////////////////////////////////

    class progress_indicator_terminal : public progress_indicator_base {

    public:

	////////////////////////////////////////////////////////////////////
	///
	///  Initializes the indicator.
	///
	///  \param  title        The title of the progress indicator.
	///  \param  description  A text to be printed in front of the 
	///                       indicator.
	///  \param  minRange     The lower bound of the range.
	///  \param  maxRange     The upper bound of the range.
	///  \param  stepValue    The increment for each step.
	///
	////////////////////////////////////////////////////////////////////

	progress_indicator_terminal(const std::string& title, 
								const std::string& description, 
								stream_offset_type minRange, 
								stream_offset_type maxRange, 
								stream_offset_type stepValue) : 
	    progress_indicator_base(title, description, minRange, maxRange, stepValue), m_title(""), m_description("") {
			m_title = title;
			m_description = description;
	}

  ////////////////////////////////////////////////////////////////////
  ///  Copy-constructor.
  ////////////////////////////////////////////////////////////////////

	progress_indicator_terminal(const progress_indicator_terminal& other) : 
	    progress_indicator_base(other), m_title(NULL), m_description(NULL) {
	    *this = other;
	}

  ////////////////////////////////////////////////////////////////////
  ///  Assignment operator.
  ////////////////////////////////////////////////////////////////////

	progress_indicator_terminal& operator=(const progress_indicator_terminal& other) 
	{
	    if (this != &other) 
		{
			progress_indicator_base::operator=(other);
			m_title = other.m_title;
			m_description = other.m_description;
		}
	    return *this;
	}

	////////////////////////////////////////////////////////////////////
	///
	///  The destructor. Free the space allocated for title/description.
	///
	////////////////////////////////////////////////////////////////////

	virtual ~progress_indicator_terminal() 
	{};
    
	////////////////////////////////////////////////////////////////////
	///
	///  Advance the indicator to the end and print an (optional)
	///  message that is followed by a newline.
	///
	///  \param  text  The message to be printed at the end of the
	///                indicator.
	///
	////////////////////////////////////////////////////////////////////

	void done(const std::string& text = std::string()) {
	    m_current = m_maxRange;
	    refresh();
	    if (!text.empty()) {
		std::cout << " " << text;
	    }
	    std::cout << std::endl;
	}

	////////////////////////////////////////////////////////////////////
	///
	///  Reset the counter. The current position is reset to the
	///  lower bound of the counting range.
	///
	////////////////////////////////////////////////////////////////////

	virtual void reset() {
	    m_current = m_minRange;
	}

	////////////////////////////////////////////////////////////////////
	///
	///  Set the lower bound of the counting range. This method
	///  also implies a reset of the counter. In order to be able
	///  to set the lower bound independent of setting the upper bound,
	///  no range checking is done.
	///
	///  \param  minRange  The new lower bound.
	///
	////////////////////////////////////////////////////////////////////

	void set_min_range(stream_offset_type minRange) {
	    m_minRange = minRange;
	    reset();
	}

	////////////////////////////////////////////////////////////////////
	///
	///  Set the upper bound of the counting range. This method
	///  also implies a reset of the counter. In order to be able
	///  to set the uper bound independent of setting the lower bound,
	///  no range checking is done.
	///
	///  \param  maxRange  The new upper bound.
	///
	////////////////////////////////////////////////////////////////////

	void set_max_range(stream_offset_type maxRange) {
	    m_maxRange = maxRange;
	    reset();
	}

	////////////////////////////////////////////////////////////////////
	///
	///  Set the increment by which the counter is advanced upon each
	///  call to step(). In order to be able to reset the counter,
	///  no range checking is done.
	///
	///  \param  stepValue  The incerement.
	///
	////////////////////////////////////////////////////////////////////

	void set_step_value(stream_offset_type stepValue) {
	    m_stepValue = stepValue;
	}
  
	////////////////////////////////////////////////////////////////////
	///
	///  Set the title of a new task to be monitored. The terminal
	///  line will be newline'd, and the title will be followed by a
	///  newline as well.
	///
	///  \param  title  The title of the new task to be monitored.
	///
	////////////////////////////////////////////////////////////////////

	void set_title(const std::string& title) 
	{
	    m_title = title;
	    std::cout << std::endl << title << std::endl;
	}

	////////////////////////////////////////////////////////////////////
	///
	///  Set the description of the task currently being monitored.
	///  Invoking this method will clear the terminal line.
	///
	///  \param  description  The decription of the task being monitored.
	///
	////////////////////////////////////////////////////////////////////

	void set_description(const std::string& description) 
	{
	    m_description = description;
	    std::cout << "\r";
	    for (int i = 0; i < 78; i++) std::cout << " ";
	    std::cout << "\r" << description << std::flush;
	}

	////////////////////////////////////////////////////////////////////
	///
	///  Display the indicator.
	///
	////////////////////////////////////////////////////////////////////

	virtual void refresh() {
	    std::cout << "\r" << m_description << " ";
	    display_percentage();
	    std::cout << std::flush;
	}

    protected:


	////////////////////////////////////////////////////////////////////
	///
	///  Compute and print the percentage or step count.
	///
	////////////////////////////////////////////////////////////////////

	void display_percentage() 
	{
		if (m_percentageUnit) {
			std::cout << std::setw(6) << std::setiosflags(std::ios::fixed) << std::setprecision(2) 
				 << ((static_cast<double>(m_current) * 100.0) / 
					 static_cast<double>(m_percentageUnit))
				 << "%";
	    }
	    else {
			std::cout << m_current << "/" << m_maxRange-m_minRange;
	    }
	}

	/**  A string holding the description of the title */
	std::string m_title;

	/**  A string holding the description of the current task */
	std::string m_description;

    private:

  ////////////////////////////////////////////////////////////////////
  /// Empty constructor.
  ////////////////////////////////////////////////////////////////////
  progress_indicator_terminal();
    };

}  //  tpie namespace

#endif // _PROGRESS_INDICATOR_TERMINAL
