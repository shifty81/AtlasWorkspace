//! atlas_audio — Audio clips, sources, mixer, and device.
//!
//! Mirrors the C++ NF::Audio system: AudioClip, AudioSource, AudioMixer,
//! AudioListener, and AudioDevice.

use serde::{Deserialize, Serialize};

// ── AudioFormat ───────────────────────────────────────────────────────────────

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum AudioFormat {
    Mono8,
    Mono16,
    Stereo8,
    Stereo16,
}

impl AudioFormat {
    pub fn bytes_per_sample(self) -> usize {
        match self {
            AudioFormat::Mono8 | AudioFormat::Stereo8 => 1,
            AudioFormat::Mono16 | AudioFormat::Stereo16 => 2,
        }
    }

    pub fn channels(self) -> usize {
        match self {
            AudioFormat::Mono8 | AudioFormat::Mono16 => 1,
            AudioFormat::Stereo8 | AudioFormat::Stereo16 => 2,
        }
    }
}

// ── AudioClip ─────────────────────────────────────────────────────────────────

/// An in-memory audio clip (raw PCM data).
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct AudioClip {
    pub name:        String,
    pub format:      AudioFormat,
    pub sample_rate: u32,
    data:            Vec<u8>,
}

impl AudioClip {
    pub fn new(name: impl Into<String>, format: AudioFormat, sample_rate: u32, data: Vec<u8>) -> Self {
        Self { name: name.into(), format, sample_rate, data }
    }

    pub fn data(&self) -> &[u8] { &self.data }
    pub fn size_bytes(&self) -> usize { self.data.len() }

    pub fn duration(&self) -> f32 {
        if self.sample_rate == 0 { return 0.0; }
        let bytes_per_frame = self.format.bytes_per_sample() * self.format.channels();
        let total_frames = self.data.len() / bytes_per_frame.max(1);
        total_frames as f32 / self.sample_rate as f32
    }
}

// ── AudioSource ───────────────────────────────────────────────────────────────

/// A single playback voice attached to an AudioClip.
#[derive(Debug, Clone, Default)]
pub struct AudioSource {
    clip:              Option<usize>,   // index into AudioDevice::clips
    playing:           bool,
    looping:           bool,
    volume:            f32,
    pitch:             f32,
    playback_position: f32,
    /// World-space position for 3-D spatial audio.
    pub position: [f32; 3],
}

impl AudioSource {
    pub fn new() -> Self {
        Self { volume: 1.0, pitch: 1.0, ..Default::default() }
    }

    pub fn with_clip(clip_index: usize) -> Self {
        Self { clip: Some(clip_index), volume: 1.0, pitch: 1.0, ..Default::default() }
    }

    pub fn play(&mut self) { self.playing = true; self.playback_position = 0.0; }
    pub fn stop(&mut self) { self.playing = false; self.playback_position = 0.0; }
    pub fn pause(&mut self) { self.playing = false; }

    pub fn set_clip(&mut self, index: usize) { self.clip = Some(index); }
    pub fn set_volume(&mut self, v: f32) { self.volume = v.clamp(0.0, 1.0); }
    pub fn set_pitch(&mut self, p: f32) { self.pitch = p.max(0.01); }
    pub fn set_looping(&mut self, looping: bool) { self.looping = looping; }

    pub fn clip_index(&self) -> Option<usize> { self.clip }
    pub fn is_playing(&self) -> bool { self.playing }
    pub fn volume(&self) -> f32 { self.volume }
    pub fn pitch(&self) -> f32 { self.pitch }
    pub fn is_looping(&self) -> bool { self.looping }
    pub fn playback_position(&self) -> f32 { self.playback_position }

    /// Advance playback time by `dt` seconds, stopping or looping as needed.
    pub fn advance(&mut self, dt: f32, clip_duration: f32) {
        if !self.playing { return; }
        self.playback_position += dt * self.pitch;
        if self.playback_position >= clip_duration {
            if self.looping {
                self.playback_position = 0.0;
            } else {
                self.playing = false;
                self.playback_position = 0.0;
            }
        }
    }
}

// ── AudioMixer ────────────────────────────────────────────────────────────────

/// A named mixer channel with volume and mute controls.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct MixerChannel {
    pub name:   String,
    pub volume: f32,
    pub muted:  bool,
}

/// Multi-channel audio mixer — maps sources to named channels.
#[derive(Debug, Clone, Default)]
pub struct AudioMixer {
    channels:       Vec<MixerChannel>,
    master_volume:  f32,
}

impl AudioMixer {
    pub fn new() -> Self {
        Self { channels: Vec::new(), master_volume: 1.0 }
    }

    pub fn add_channel(&mut self, name: impl Into<String>, volume: f32) -> usize {
        let id = self.channels.len();
        self.channels.push(MixerChannel { name: name.into(), volume: volume.clamp(0.0, 1.0), muted: false });
        id
    }

    pub fn set_channel_volume(&mut self, id: usize, vol: f32) {
        if let Some(ch) = self.channels.get_mut(id) {
            ch.volume = vol.clamp(0.0, 1.0);
        }
    }

    pub fn set_channel_muted(&mut self, id: usize, muted: bool) {
        if let Some(ch) = self.channels.get_mut(id) { ch.muted = muted; }
    }

    pub fn effective_volume(&self, id: usize) -> f32 {
        self.channels.get(id).map(|ch| {
            if ch.muted { 0.0 } else { ch.volume * self.master_volume }
        }).unwrap_or(0.0)
    }

    pub fn set_master_volume(&mut self, vol: f32) { self.master_volume = vol.clamp(0.0, 1.0); }
    pub fn master_volume(&self) -> f32 { self.master_volume }
    pub fn channel_count(&self) -> usize { self.channels.len() }
    pub fn channels(&self) -> &[MixerChannel] { &self.channels }
}

// ── AudioListener ─────────────────────────────────────────────────────────────

/// Represents the listener's position and orientation for spatial audio.
#[derive(Debug, Clone)]
pub struct AudioListener {
    pub position: [f32; 3],
    pub forward:  [f32; 3],
    pub up:       [f32; 3],
}

impl Default for AudioListener {
    fn default() -> Self {
        Self {
            position: [0.0; 3],
            forward:  [0.0, 0.0, -1.0],
            up:       [0.0, 1.0, 0.0],
        }
    }
}

// ── AudioDevice ───────────────────────────────────────────────────────────────

/// Main audio subsystem — manages clips, sources, mixer, and listener.
pub struct AudioDevice {
    clips:    Vec<AudioClip>,
    sources:  Vec<AudioSource>,
    mixer:    AudioMixer,
    listener: AudioListener,
}

impl AudioDevice {
    pub fn new() -> Self {
        Self {
            clips:    Vec::new(),
            sources:  Vec::new(),
            mixer:    AudioMixer::new(),
            listener: AudioListener::default(),
        }
    }

    pub fn init(&mut self) -> bool {
        self.mixer.add_channel("Master", 1.0);
        self.mixer.add_channel("SFX",    1.0);
        self.mixer.add_channel("Music",  0.8);
        self.mixer.add_channel("Voice",  1.0);
        tracing::info!(target: "Audio", "Audio device initialized");
        true
    }

    pub fn shutdown(&mut self) {
        self.sources.clear();
        self.clips.clear();
        tracing::info!(target: "Audio", "Audio device shutdown");
    }

    pub fn update(&mut self, dt: f32) {
        for src in &mut self.sources {
            if let Some(clip_idx) = src.clip_index() {
                let dur = self.clips.get(clip_idx).map(|c| c.duration()).unwrap_or(0.0);
                src.advance(dt, dur);
            }
        }
    }

    pub fn add_clip(&mut self, clip: AudioClip) -> usize {
        let idx = self.clips.len();
        self.clips.push(clip);
        idx
    }

    pub fn add_source(&mut self) -> usize {
        let idx = self.sources.len();
        self.sources.push(AudioSource::new());
        idx
    }

    pub fn source(&self, idx: usize) -> Option<&AudioSource> { self.sources.get(idx) }
    pub fn source_mut(&mut self, idx: usize) -> Option<&mut AudioSource> { self.sources.get_mut(idx) }
    pub fn clip(&self, idx: usize) -> Option<&AudioClip> { self.clips.get(idx) }

    pub fn set_listener(&mut self, listener: AudioListener) { self.listener = listener; }
    pub fn listener(&self) -> &AudioListener { &self.listener }
    pub fn mixer(&self) -> &AudioMixer { &self.mixer }
    pub fn mixer_mut(&mut self) -> &mut AudioMixer { &mut self.mixer }

    pub fn clip_count(&self) -> usize { self.clips.len() }
    pub fn source_count(&self) -> usize { self.sources.len() }
}

impl Default for AudioDevice {
    fn default() -> Self { Self::new() }
}

// ── Tests ─────────────────────────────────────────────────────────────────────

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn audio_clip_duration_pcm_stereo() {
        // Stereo16: bytes_per_sample=2, channels=2 → 4 bytes per frame
        // 44100 Hz, 1 second → 44100*4 = 176400 bytes
        let data = vec![0u8; 176_400];
        let clip = AudioClip::new("test", AudioFormat::Stereo16, 44100, data);
        let dur = clip.duration();
        assert!((dur - 1.0).abs() < 1e-3, "stereo clip duration should be 1 second, got {dur}");
    }

    #[test]
    fn audio_source_play_stop() {
        let mut src = AudioSource::new();
        assert!(!src.is_playing());
        src.play();
        assert!(src.is_playing());
        src.stop();
        assert!(!src.is_playing());
        assert_eq!(src.playback_position(), 0.0);
    }

    #[test]
    fn audio_source_advance_without_loop_stops_at_end() {
        let mut src = AudioSource::with_clip(0);
        src.play();
        src.set_looping(false);
        src.advance(2.0, 1.0);  // advance 2 s on a 1 s clip → should stop
        assert!(!src.is_playing(), "non-looping source should stop when clip ends");
    }

    #[test]
    fn audio_source_advance_with_loop_wraps() {
        let mut src = AudioSource::with_clip(0);
        src.play();
        src.set_looping(true);
        src.advance(1.5, 1.0);  // advance 1.5 s on a 1 s clip → wrap to 0.5
        assert!(src.is_playing(), "looping source must keep playing");
        assert!(src.playback_position() < 1.0, "position must wrap");
    }

    #[test]
    fn audio_source_volume_clamp() {
        let mut src = AudioSource::new();
        src.set_volume(2.0);
        assert_eq!(src.volume(), 1.0, "volume clamps at 1.0");
        src.set_volume(-0.5);
        assert_eq!(src.volume(), 0.0, "volume clamps at 0.0");
    }

    #[test]
    fn mixer_effective_volume() {
        let mut mixer = AudioMixer::new();
        mixer.set_master_volume(0.5);
        let ch = mixer.add_channel("Music", 0.8);
        let eff = mixer.effective_volume(ch);
        assert!((eff - 0.4).abs() < 1e-5, "effective = master * channel: 0.5*0.8 = 0.4");
    }

    #[test]
    fn mixer_mute_channel() {
        let mut mixer = AudioMixer::new();
        let ch = mixer.add_channel("SFX", 1.0);
        mixer.set_channel_muted(ch, true);
        assert_eq!(mixer.effective_volume(ch), 0.0, "muted channel must have zero effective volume");
    }

    #[test]
    fn audio_device_init_and_shutdown() {
        let mut device = AudioDevice::new();
        let ok = device.init();
        assert!(ok, "headless audio device init should succeed");
        device.shutdown();
    }

    #[test]
    fn audio_device_add_clip_and_source() {
        let mut device = AudioDevice::new();
        device.init();
        let clip = AudioClip::new("click", AudioFormat::Mono16, 44100, vec![0u8; 1000]);
        let _clip_idx = device.add_clip(clip);
        let src_idx = device.add_source();
        assert!(device.source(src_idx).is_some());
    }
}
