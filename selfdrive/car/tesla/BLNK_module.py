class Blinker:
    def __init__(self):
        self.tap_duration_frames = (
            55  # stalk signal for less than 550ms means it was tapped
        )
        self.tap_direction = 0  # tap direction lasts for one frame (when it was let go)
        self.blinker_on_frame_start = 0
        self.override_frame_end = 0
        self.override_direction = 0
        self.prev_turn_signal_stalk_state = 0

    def update_state(self, CS, frame):
        self.tap_direction = 0
        if CS.turn_signal_stalk_state > 0 and self.prev_turn_signal_stalk_state == 0:
            if self.override_frame_end == 0:
                self.blinker_on_frame_start = frame
        elif (
            CS.turn_signal_stalk_state == 0 and self.prev_turn_signal_stalk_state > 0
        ):  # turn signal stalk just turned off
            if frame - self.blinker_on_frame_start <= self.tap_duration_frames:
                self.tap_direction = self.prev_turn_signal_stalk_state
                
        if 0 < self.override_frame_end < frame or (
            self.override_direction > 0
            and (CS.turn_signal_stalk_state > 0 or self.override_frame_end == 0)
        ):
            self.override_frame_end = 0
            self.override_direction = 0
        if (
            self.blinker_on_frame_start > 0
            and CS.turn_signal_stalk_state == 0
            and not CS.turn_signal_blinking
            and self.override_frame_end == 0
        ):
            self.blinker_on_frame_start = 0