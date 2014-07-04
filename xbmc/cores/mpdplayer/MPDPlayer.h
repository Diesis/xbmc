#pragma once

#include <list>
#include <set>
#include "cores/IPlayer.h"
#include "threads/Thread.h"
#include "threads/SharedSection.h"
#include "utils/Job.h"
#include "cores/IAudioCallback.h"
#include "cores/AudioEngine/Utils/AEChannelInfo.h"
#include "mpd/client.h"

// fast forward declarations
class IAEStream;
class CFileItem;

class MPDPlayer : public IPlayer, public CThread, public IJobCallback
{
friend class MPDQueueNextFileJob;
public:
	MPDPlayer(IPlayerCallback& callback);
	virtual ~MPDPlayer();

	virtual void RegisterAudioCallback(IAudioCallback* pCallback);
	virtual void UnRegisterAudioCallback();
	virtual bool OpenFile(const CFileItem& file, const CPlayerOptions &options);
	virtual bool QueueNextFile(const CFileItem &file);
	virtual void OnNothingToQueueNotify();
	virtual bool CloseFile(bool reopen = false);
	virtual bool IsPlaying() const;
	virtual void Pause();
	virtual bool IsPaused() const;
	virtual bool HasVideo() const { return false; }
	virtual bool HasAudio() const { return true; }
	virtual bool CanSeek();
	virtual void Seek(bool bPlus = true, bool bLargeStep = false, bool bChapterOverride = false);
	virtual void SeekPercentage(float fPercent = 0.0f);
	virtual float GetPercentage();
	virtual void  SetVolume(float volume);
	virtual void SetDynamicRangeCompression(long drc);
	virtual void GetAudioInfo( CStdString& strAudioInfo) {}
	virtual void GetVideoInfo( CStdString& strVideoInfo) {}
	virtual void GetGeneralInfo( CStdString& strVideoInfo) {}
	virtual void ToFFRW(int iSpeed = 0);
	virtual int GetCacheLevel() const;
	virtual int64_t GetTotalTime();
	virtual void GetAudioStreamInfo(int index, SPlayerAudioStreamInfo &info);
	virtual int64_t GetTime();
	virtual void SeekTime(int64_t iTime = 0);
	virtual bool SkipNext();

	static bool HandlesType(const CStdString &type);
	virtual void OnJobComplete(unsigned int jobID, bool success, CJob *job);

	//void Update(bool bPauseDrawing = false) {}
	//int GetAudioBitrate();
	//virtual int GetChannels() = 0;
	//int GetBitsPerSample();
	//int GetSampleRate();
	//virtual bool  ControlsVolume() {return false;}
	//virtual void  SetMute(bool bOnOff);
	//CStdString GetAudioCodecName();

	// to test
	// virtual bool  IsPassthrough() {return true;};

	//static bool HandlesType(const CStdString &type);
	//virtual void OnJobComplete(unsigned int jobID, bool success, CJob *job);

	struct {
		char         m_codec[21];
		int64_t      m_time;
		int64_t      m_totalTime;
		int          m_channelCount;
		int          m_bitsPerSample;
		int          m_sampleRate;
		int          m_audioBitrate;
		int          m_cacheLevel;
		bool         m_canSeek;
	} m_playerGUIData;

protected:
	virtual void OnStartup() {}
	virtual void Process();
	virtual void OnExit();

private:
	typedef struct {
		struct			mpd_song* m_song;
		const struct	mpd_audio_format* m_audio_format;
		CStdString		m_fileUri;
		unsigned int	m_duration;
		int				m_songId;			/* the songId from MPD */
		int				m_songIdFromXbmc;	/* the songId from the XBMC playlist */
		unsigned int	m_mpdCurSongId;	/* the current songId from MPD*/
		unsigned int	mpdstatussongid;
		int64_t			m_startOffset;         /* the stream start offset */
		int64_t			m_endOffset;           /* the stream end offset */
		int				m_channelCount;        /* channel layout information */
		unsigned int	m_sampleRate;          /* sample rate of the stream */
		unsigned int	m_encodedSampleRate;   /* the encoded sample rate of raw streams */
		// enum AEDataFormat m_dataFormat;          /* data format of the samples */
		unsigned int	m_bytesPerSample;      /* number of bytes per audio sample */
		unsigned int	m_bytesPerFrame;       /* number of bytes per audio frame */
		bool				m_started;             /* if playback of this stream has been started */
		bool				m_finished;            /* if this stream is finishing */
		int				m_framesSent;          /* number of frames sent to the stream */
		int				m_prepareNextAtFrame;  /* when to prepare the next stream */
		bool				m_prepareTriggered;    /* if the next stream has been prepared */
		int				m_playNextAtFrame;     /* when to start playing the next stream */
		bool				m_playNextTriggered;   /* if this stream has started the next one */
		bool				m_fadeOutTriggered;    /* if the stream has been told to fade out */
		int				m_seekNextAtFrame;     /* the FF/RR sample to seek at */
		int				m_seekFrame;           /* the exact position to seek too, -1 for none */
		IAEStream*		m_stream;              /* the playback stream */
		float				m_volume;              /* the initial volume level to set the stream to on creation */
		bool				m_canPlay;
		bool				m_isSlaved;            /* true if the stream has been slaved to another */
		CStdString		m_codec; 
	} StreamInfo;

	typedef std::list<StreamInfo*> StreamList;

	bool                m_signalSpeedChange;   /* true if OnPlaybackSpeedChange needs to be called */
	int                 m_playbackSpeed;       /* the playback speed (1 = normal) */
	bool                m_isPlaying;
	bool                m_isPaused;
	bool                m_isFinished;
	unsigned int        m_defaultCrossfadeMS;  /* how long the default crossfade is in ms */
	unsigned int        m_upcomingCrossfadeMS; /* how long the upcoming crossfade is in ms */
	StreamInfo*         m_currentStream;       /* the current playing stream */
	IAudioCallback*     m_audioCallback;       /* the viz audio callback */
	CFileItem*          m_FileItem;            /* our queued file or current file if no file is queued */

	CEvent              m_startEvent;          /* event for playback start */

	CSharedSection      m_streamsLock;         /* lock for the stream list */
	StreamList          m_streams;             /* playing streams */
	int                 m_jobCounter;
	CEvent              m_jobEvent;

	StreamList          m_finishing;           /* finishing streams */

	bool                m_queueIsFinished;     /* if there are no more songs in the queue */
	bool QueueNextFileEx(const CFileItem &file, bool fadeIn = true, bool job = false);
	void ProcessStreams();
	bool ProcessStream(StreamInfo *si);
	int64_t GetTotalTime64();
	void UpdateCrossfadeTime(const CFileItem& file);
	void UpdateStreamInfoPlayNextAtFrame(StreamInfo *si, unsigned int crossFadingTime);
	void UpdateGUIData(StreamInfo *si);
	int64_t GetTimeInternal();
	void MPDDisconnect();
	int  ChannelCount();
	void UnPause();
	int m_channelCount;
	struct mpd_connection *m_connection;
	struct mpd_connection *m_connection_for_status;
	struct mpd_status *m_status;
	unsigned int  m_current_volume;
	bool m_current_mute;
	bool m_change_volume;
	bool CheckIfMounted(CStdString& mountpoint);
	CStdString SMBMountPoint(CStdString &m_strName);
	int mpd_next_song_in_playlist;

	//
	// MPD stuff
	//
	bool MPDCheckForErrors();
	bool MPDClear();
	bool MPDPlay();
	bool MPDStatusFree();
	bool MPDStop();
	bool MPDPause();
	bool MountSMBShare(CStdString& mainFile, StreamInfo* si);
};

