#ifndef COURSE_H
#define COURSE_H
/*
-----------------------------------------------------------------------------
 Class: Course

 Desc: A queue of songs and notes.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "PlayerNumber.h"
#include "GameConstantsAndTypes.h"

struct PlayerOptions;
struct SongOptions;
class Song;
class Steps;

enum CourseEntryType
{
	COURSE_ENTRY_FIXED, 
	COURSE_ENTRY_RANDOM, 
	COURSE_ENTRY_RANDOM_WITHIN_GROUP, 
	COURSE_ENTRY_BEST, 
	COURSE_ENTRY_WORST,
	NUM_COURSE_ENTRY_TYPES	// leave this at the end
};

inline CString CourseEntryTypeToString( CourseEntryType cet )
{
	switch( cet )
	{
	case COURSE_ENTRY_FIXED:				return "fixed";
	case COURSE_ENTRY_RANDOM:				return "random";
	case COURSE_ENTRY_RANDOM_WITHIN_GROUP:	return "random_within_group";
	case COURSE_ENTRY_BEST:					return "best";
	case COURSE_ENTRY_WORST:				return "worst";
	default:					ASSERT(0);	return "";
	}
}

class CourseEntry
{
public:
	CourseEntryType type;
	bool mystery;			// show "??????"
	Song* pSong;			// used in type=fixed
	CString group_name;		// used in type=random_within_group
	Difficulty difficulty;	// = DIFFICULTY_INVALID if no difficulty specified
	int low_meter;			// = -1 if no meter range specified
	int high_meter;			// = -1 if no meter range specified
	int players_index;		// ignored if type isn't 'best' or 'worst'
	CString modifiers;		// set player and song options using these

	CourseEntry()
	{
		type = (CourseEntryType)0;
		mystery = false;
		pSong = NULL;
		group_name = "";
		difficulty = DIFFICULTY_INVALID;
		low_meter = -1;
		high_meter = -1;
		players_index = 0;
		modifiers = "";
	}
};

class Course
{
public:
	Course();

	bool		m_bIsAutogen;		// was this created by AutoGen?
	CString		m_sPath;
	CString		m_sName;
	CString		m_sTranslitName;	// used for unlocks

	bool HasBanner() const;

	CString		m_sBannerPath;
	CString		m_sCDTitlePath;

	bool		m_bRepeat;	// repeat after last song?  "Endless"
	bool		m_bRandomize;	// play the songs in a random order
	bool		m_bDifficult; // only make something difficult once
	int			m_iLives;	// -1 means use bar life meter
	int			m_iMeter;	// -1 autogens

	vector<CourseEntry> m_entries;

	struct Info
	{
		Info(): pSong(NULL), pNotes(NULL), Random(false), Difficult(false) { }
		Song*	pSong;
		Steps*	pNotes;
		CString	Modifiers;
		bool	Random;
		bool	Mystery;
		bool	Difficult; /* set to true if this is the difficult version */
		/* Corresponding entry in m_entries: */
		int		CourseIndex;
	};

	// Dereferences course_entries and returns only the playable Songs and Steps
	void GetCourseInfo( StepsType nt, vector<Info> &ci, int Difficult = -1 ) const;

	int GetEstimatedNumStages() const { return m_entries.size(); }
	bool HasDifficult( StepsType nt ) const;
	bool IsPlayableIn( StepsType nt ) const;
	bool CourseHasBestOrWorst() const;
	RageColor GetColor() const;
	Difficulty GetDifficulty( const Info &stage ) const;
	void GetMeterRange( const Info &stage, int& iMeterLowOut, int& iMeterHighOut ) const;
	bool GetTotalSeconds( float& fSecondsOut ) const;

	bool IsNonstop() const { return GetPlayMode() == PLAY_MODE_NONSTOP; }
	bool IsOni() const { return GetPlayMode() == PLAY_MODE_ONI; }
	bool IsEndless() const { return GetPlayMode() == PLAY_MODE_ENDLESS; }
	PlayMode GetPlayMode() const;
	float GetMeter( int Difficult = -1 ) const;

	bool IsFixed() const;

	void LoadFromCRSFile( CString sPath );
	void Save();
	void AutogenEndlessFromGroup( CString sGroupName, vector<Song*> &apSongsInGroup );
	void AutogenNonstopFromGroup( CString sGroupName, vector<Song*> &apSongsInGroup );


	struct MemCardData
	{
		MemCardData()
		{
			iNumTimesPlayed = 0;
		}

		int iNumTimesPlayed;

		struct HighScore
		{
			int iScore;
			float fSurviveTime;
			CString	sName;

			HighScore()
			{
				iScore = 0;
				fSurviveTime = 0;
			}

			bool operator>=( const HighScore& other ) const
			{
				return iScore >= other.iScore;
			}
		};
		vector<HighScore> vHighScores;

		void AddHighScore( HighScore hs, int &iIndexOut );

		HighScore GetTopScore()
		{
			if( vHighScores.empty() )
				return HighScore();
			else
				return vHighScores[0];
		}

	} m_MemCardDatas[NUM_STEPS_TYPES][NUM_MEMORY_CARDS];
	
	void AddHighScore( StepsType st, PlayerNumber pn, MemCardData::HighScore hs, int &iPersonalIndexOut, int &iMachineIndexOut )
	{
		hs.sName = RANKING_TO_FILL_IN_MARKER[pn];
		m_MemCardDatas[st][pn].AddHighScore( hs, iPersonalIndexOut );
		m_MemCardDatas[st][MEMORY_CARD_MACHINE].AddHighScore( hs, iMachineIndexOut );
	}

	MemCardData::HighScore GetTopScore( StepsType st, MemoryCard card )
	{
		return m_MemCardDatas[st][card].GetTopScore();
	}

	int GetNumTimesPlayed( StepsType st, MemoryCard card )
	{
		return m_MemCardDatas[st][card].iNumTimesPlayed;
	}

	// sorting values
	int		SortOrder_TotalDifficulty;
	int		SortOrder_Ranking;

	void UpdateCourseStats();

private:
	void GetMeterRange( int stage, int& iMeterLowOut, int& iMeterHighOut, int Difficult = -1 ) const;
};


void SortCoursePointerArrayByDifficulty( vector<Course*> &apCourses );
void SortCoursePointerArrayByType( vector<Course*> &apCourses );
void SortCoursePointerArrayByAvgDifficulty( vector<Course*> &apCourses );
void SortCoursePointerArrayByTotalDifficulty( vector<Course*> &apCourses );
void SortCoursePointerArrayByRanking( vector<Course*> &apCourses );

void MoveRandomToEnd( vector<Course*> &apCourses );

#endif
