#include "stdafx.h"
/*
-----------------------------------------------------------------------------
 Class: ScreenRanking

 Desc: See header.

 Copyright (c) 2001-2002 by the person(s) listed below.  All rights reserved.
	Chris Danford
-----------------------------------------------------------------------------
*/

#include "ScreenRanking.h"
#include "ThemeManager.h"
#include "SongManager.h"
#include "GameState.h"
#include "GameManager.h"


#define CATEGORY_X				THEME->GetMetricF("ScreenRanking","CategoryX")
#define CATEGORY_Y				THEME->GetMetricF("ScreenRanking","CategoryY")
#define TYPE_X					THEME->GetMetricF("ScreenRanking","TypeX")
#define TYPE_Y					THEME->GetMetricF("ScreenRanking","TypeY")
#define LINE_SPACING_X			THEME->GetMetricF("ScreenRanking","LineSpacingX")
#define LINE_SPACING_Y			THEME->GetMetricF("ScreenRanking","LineSpacingY")
#define BULLETS_START_X			THEME->GetMetricF("ScreenRanking","BulletsStartX")
#define BULLETS_START_Y			THEME->GetMetricF("ScreenRanking","BulletsStartY")
#define NAMES_START_X			THEME->GetMetricF("ScreenRanking","NamesStartX")
#define NAMES_START_Y			THEME->GetMetricF("ScreenRanking","NamesStartY")
#define NAMES_ZOOM				THEME->GetMetricF("ScreenRanking","NamesZoom")
#define NAMES_COLOR				THEME->GetMetricC("ScreenRanking","NamesColor")
#define SCORES_START_X			THEME->GetMetricF("ScreenRanking","ScoresStartX")
#define SCORES_START_Y			THEME->GetMetricF("ScreenRanking","ScoresStartY")
#define SCORES_ZOOM				THEME->GetMetricF("ScreenRanking","ScoresZoom")
#define SCORES_COLOR			THEME->GetMetricC("ScreenRanking","ScoresColor")
#define SECONDS_PER_PAGE		THEME->GetMetricF("ScreenRanking","SecondsPerPage")
#define SHOW_CATEGORIES			THEME->GetMetricB("ScreenRanking","ShowCategories")
#define COURSES_TO_SHOW			THEME->GetMetric("ScreenRanking","CoursesToShow")
#define NOTES_TYPES_TO_HIDE		THEME->GetMetric("ScreenRanking","NotesTypesToHide")


const ScreenMessage SM_ShowNextPage		=	(ScreenMessage)(SM_User+67);
const ScreenMessage SM_HidePage			=	(ScreenMessage)(SM_User+68);


ScreenRanking::ScreenRanking() : ScreenAttract("ScreenRanking","ranking")
{
	m_textCategory.LoadFromFont( THEME->GetPathTo("Fonts","Header2") );
	m_textCategory.TurnShadowOff();
	m_textCategory.SetXY( CATEGORY_X, CATEGORY_Y );
	this->AddChild( &m_textCategory );

	m_textType.LoadFromFont( THEME->GetPathTo("Fonts","Header2") );
	m_textType.TurnShadowOff();
	m_textType.SetXY( TYPE_X, TYPE_Y );
	this->AddChild( &m_textType );

	for( int i=0; i<NUM_HIGH_SCORE_LINES; i++ )
	{
		m_sprBullets[i].Load( THEME->GetPathTo("Graphics",("ranking bullets 1x5")) );
		m_sprBullets[i].SetXY( BULLETS_START_X+LINE_SPACING_X*i, BULLETS_START_Y+LINE_SPACING_Y*i );
		m_sprBullets[i].StopAnimating();
		m_sprBullets[i].SetState(i);
		m_sprBullets[i].SetDiffuse( RageColor(1,1,1,0) );
		this->AddChild( &m_sprBullets[i] );

		m_textNames[i].LoadFromFont( THEME->GetPathTo("Fonts","ranking") );
		m_textNames[i].TurnShadowOff();
		m_textNames[i].SetXY( NAMES_START_X+LINE_SPACING_X*i, NAMES_START_Y+LINE_SPACING_Y*i );
		m_textNames[i].SetZoom( NAMES_ZOOM );
		m_textNames[i].SetDiffuse( NAMES_COLOR );
		this->AddChild( &m_textNames[i] );

		m_textScores[i].LoadFromFont( THEME->GetPathTo("Fonts","ranking") );
		m_textScores[i].TurnShadowOff();
		m_textScores[i].SetXY( SCORES_START_X+LINE_SPACING_X*i, SCORES_START_Y+LINE_SPACING_Y*i );
		m_textScores[i].SetZoom( SCORES_ZOOM );
		m_textScores[i].SetDiffuse( SCORES_COLOR );
		this->AddChild( &m_textScores[i] );
	}


	vector<NotesType> aNotesTypesToShow;
	GAMEMAN->GetNotesTypesForGame( GAMESTATE->m_CurGame, aNotesTypesToShow );

	// subtract hidden NotesTypes
	{
		vector<CString> asNotesTypesToHide;
		split( NOTES_TYPES_TO_HIDE, ",", asNotesTypesToHide, true );
		for( unsigned i=0; i<asNotesTypesToHide.size(); i++ )
		{
			NotesType nt = GameManager::StringToNotesType(asNotesTypesToHide[i]);
			if( nt != NOTES_TYPE_INVALID )
			{
				const vector<NotesType>::iterator iter = find( aNotesTypesToShow.begin(), aNotesTypesToShow.end(), nt );
				if( iter != aNotesTypesToShow.end() )
					aNotesTypesToShow.erase( iter );
			}
		}
	}


	// fill m_vPagesToShow
	if( SHOW_CATEGORIES )
	{
		for( unsigned i=0; i<aNotesTypesToShow.size(); i++ )
		{
			for( int c=0; c<NUM_RANKING_CATEGORIES; c++ )
			{
				PageToShow pts;
				pts.type = PageToShow::TYPE_CATEGORY;
				pts.category = (RankingCategory)c;
				pts.nt = aNotesTypesToShow[i];
				m_vPagesToShow.push_back( pts );
			}
		}
	}

	vector<CString> asCoursePaths;
	split( COURSES_TO_SHOW, ",", asCoursePaths, true );
	for( unsigned i=0; i<aNotesTypesToShow.size(); i++ )
	{
		for( unsigned c=0; c<asCoursePaths.size(); c++ )
		{
			PageToShow pts;
			pts.type = PageToShow::TYPE_COURSE;
			pts.nt = aNotesTypesToShow[i];
			pts.pCourse = SONGMAN->GetCourseFromPath( asCoursePaths[c] );
			if( pts.pCourse )
				m_vPagesToShow.push_back( pts );
		}
	}

	this->MoveToTail( &m_Fade );

	this->ClearMessageQueue( SM_BeginFadingOut );	// ignore ScreenAttract's SecsToShow

	this->SendScreenMessage( SM_ShowNextPage, 0.5f );
}

void ScreenRanking::HandleScreenMessage( const ScreenMessage SM )
{
	switch( SM )
	{
	case SM_BeginFadingOut:
		m_Fade.CloseWipingRight(SM_GoToNextScreen);
		break;
	case SM_ShowNextPage:
		if( m_vPagesToShow.size() > 0 )
		{
			SetPage( m_vPagesToShow[0] );
			m_vPagesToShow.erase( m_vPagesToShow.begin() );
			this->SendScreenMessage( SM_HidePage, SECONDS_PER_PAGE-1 );
			TweenPageOnScreen();
		}
		else
		{
			m_Fade.CloseWipingRight(SM_GoToNextScreen);
		}
		break;
	case SM_HidePage:
		TweenPageOffScreen();
		this->SendScreenMessage( SM_ShowNextPage, 1 );
		break;
	}

	ScreenAttract::HandleScreenMessage( SM );
}

void ScreenRanking::SetPage( PageToShow pts )
{
	switch( pts.type )
	{
	case PageToShow::TYPE_CATEGORY:
		{
			m_textCategory.SetText( ssprintf("Type %c", 'A'+pts.category) );
			m_textType.SetText( GameManager::NotesTypeToString(pts.nt) );
			for( int l=0; l<NUM_HIGH_SCORE_LINES; l++ )
			{
				CString sName = SONGMAN->m_MachineScores[pts.nt][pts.category][l].sName;
				float fScore = SONGMAN->m_MachineScores[pts.nt][pts.category][l].fScore;
				m_textNames[l].SetText( sName );
				m_textScores[l].SetText( ssprintf("%.0f",fScore) );
			}
		}
		break;
	case PageToShow::TYPE_COURSE:
		{
			m_textCategory.SetText( pts.pCourse->m_sName );
			m_textType.SetText( GameManager::NotesTypeToString(pts.nt) );
			for( int l=0; l<NUM_HIGH_SCORE_LINES; l++ )
			{
				CString sName = pts.pCourse->m_MachineScores[pts.nt][l].sName;
				int iDancePoints = pts.pCourse->m_MachineScores[pts.nt][l].iDancePoints;
				m_textNames[l].SetText( sName );
				m_textScores[l].SetText( ssprintf("%d",iDancePoints) );
			}
		}
		break;
	default:
		ASSERT(0);
	}
}

void ScreenRanking::TweenPageOnScreen()
{
	m_textCategory.SetDiffuse( RageColor(1,1,1,1) );
	m_textCategory.FadeOn(0,"bounce right",0.5f);
	m_textType.SetDiffuse( RageColor(1,1,1,1) );
	m_textType.FadeOn(0.1f,"bounce right",0.5f);

	for( int l=0; l<NUM_HIGH_SCORE_LINES; l++ )
	{
		m_sprBullets[l].SetDiffuse( RageColor(1,1,1,1) );
		m_sprBullets[l].FadeOn(0.2f+l*0.1f,"bounce right far",1.f);
		m_textNames[l].SetDiffuse( RageColor(1,1,1,1) );
		m_textNames[l].FadeOn(0.2f+l*0.1f,"bounce right far",1.f);
		m_textScores[l].SetDiffuse( RageColor(1,1,1,1) );
		m_textScores[l].FadeOn(0.2f+l*0.1f,"bounce right far",1.f);
	}
}

void ScreenRanking::TweenPageOffScreen()
{
	m_textCategory.FadeOff(0,"fade",0.25f);
	m_textType.FadeOff(0.1f,"fade",0.25f);

	for( int l=0; l<NUM_HIGH_SCORE_LINES; l++ )
	{
		m_sprBullets[l].FadeOff(0.2f+l*0.1f,"fade",0.25f);
		m_textNames[l].FadeOff(0.2f+l*0.1f,"fade",0.25f);
		m_textScores[l].FadeOff(0.2f+l*0.1f,"fade",0.25f);
	}
}
