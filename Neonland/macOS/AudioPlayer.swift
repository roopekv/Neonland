import AVFAudio

class AudioPlayer : NSObject, AVAudioPlayerDelegate {
    static let shared = AudioPlayer()
    
    let mainAudioPlayers: [AVAudioPlayer]
    var extraAudioPlayers = [AVAudioPlayer]()
    
    static let audioIdToName: [UInt32 : String] = [
        LASER1_AUDIO.rawValue : "laser1",
        LASER2_AUDIO.rawValue : "laser2",
        LASER3_AUDIO.rawValue : "laser3",
        EXPLOSION_AUDIO.rawValue : "explosion",
        CLICK_AUDIO.rawValue : "click",
        MUSIC_AUDIO.rawValue : "neon_beat",
        LOSE_HP_AUDIO.rawValue : "lose_hp",
        GAME_OVER_AUDIO.rawValue : "game_over",
        LEVEL_CLEARED_AUDIO.rawValue : "level_cleared"
    ]
    
    static func createAudioPlayer(_ audio: AudioType) -> AVAudioPlayer {
        let ext = Neon_IsMusic(audio) ? "mp3" : "wav"
        let player = try! AVAudioPlayer(contentsOf: Bundle.main.url(forResource: audioIdToName[audio.rawValue],
                                                                    withExtension: ext)!)
        if (Neon_IsMusic(audio)) {
            player.numberOfLoops = -1
        }
        
        player.prepareToPlay()

        return player
    }
    
    private override init() {
        var audioPlayers = [AVAudioPlayer?](repeating: nil, count: Int(AudioTypeCount.rawValue))
        
        for pair in AudioPlayer.audioIdToName {
            audioPlayers[Int(pair.key)] = AudioPlayer.createAudioPlayer(AudioType(pair.key))
        }
        
        self.mainAudioPlayers = audioPlayers.map { $0! }
        
        super.init()
    }
    
    func play(_ audio: AudioType) {
        let id = audio.rawValue;
        
        if (extraAudioPlayers.count > 10) {
            return;
        }
        
        var player: AVAudioPlayer?
        
        if mainAudioPlayers[Int(id)].isPlaying {
            player = AudioPlayer.createAudioPlayer(audio)
            
            if let player {
                player.delegate = self;
                extraAudioPlayers.append(player)
            }
        }
        else {
            player = mainAudioPlayers[Int(id)]
        }
        
        if let player {
            if Neon_IsMusic(audio) {
                player.volume = Neon_MusicVolume()
            }
            else {
                player.volume = Neon_SFXVolume()
            }
            
            player.play()
        }
    }
    
    func audioPlayerDidFinishPlaying(_ player: AVAudioPlayer, successfully flag: Bool) {
        extraAudioPlayers.remove(at: extraAudioPlayers.firstIndex(of: player)!)
    }
}
