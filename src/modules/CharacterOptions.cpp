struct CharacterOptions {
  
  float extra_direction_smooth;
  float extra_velocity_smooth;
  float extra_strafe_smooth;
  float extra_crouched_smooth;
  float extra_gait_smooth;
  float extra_joint_smooth;
  CharacterOptions()
    : extra_direction_smooth(0.9)
    , extra_velocity_smooth(0.9)
    , extra_strafe_smooth(0.9)
    , extra_crouched_smooth(0.9)
    , extra_gait_smooth(0.1)
    , extra_joint_smooth(0.5)
    {}
};