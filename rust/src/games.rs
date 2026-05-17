use std::time::{SystemTime, UNIX_EPOCH};

const X_MAX: f32 = 320.0;
const Y_MAX: f32 = 230.0;

#[derive(Debug, Clone)]
struct Lcg {
    state: u64,
}

impl Lcg {
    fn seeded() -> Self {
        let nanos = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .map(|duration| duration.as_nanos() as u64)
            .unwrap_or(0x5eed_2024);
        Self {
            state: nanos ^ 0xa5a5_5a5a_1337_2024,
        }
    }

    fn next_u32(&mut self) -> u32 {
        self.state = self
            .state
            .wrapping_mul(6364136223846793005)
            .wrapping_add(1442695040888963407);
        (self.state >> 32) as u32
    }

    fn next_range(&mut self, min: u32, max: u32) -> u32 {
        debug_assert!(min <= max);
        min + (self.next_u32() % (max - min + 1))
    }

    fn next_unit(&mut self) -> f32 {
        self.next_u32() as f32 / u32::MAX as f32
    }
}

#[derive(Debug, Clone)]
struct BallState {
    x: f32,
    y: f32,
    speed_x: f32,
    speed_y: f32,
}

/// 弹球游戏：host Rust 版本用文本帧模拟 C++ TFT 弹球动画。
pub fn ball_game(radius: u16, num_balls: usize, trail: bool) -> Vec<String> {
    const GRAVITY: f32 = 0.05;
    const FLOOR_DAMP: f32 = 0.85;
    const WALL_DAMP: f32 = 0.90;

    let mut rng = Lcg::seeded();
    let radius_f = radius as f32;
    let mut balls = (0..num_balls)
        .map(|index| {
            let angle = rng.next_unit() * std::f32::consts::TAU;
            let speed = 1.5 + rng.next_unit() * 3.5;
            BallState {
                x: X_MAX / 2.0 + index as f32,
                y: Y_MAX / 2.0,
                speed_x: speed * angle.cos(),
                speed_y: speed * angle.sin(),
            }
        })
        .collect::<Vec<_>>();

    for _ in 0..12 {
        for ball in &mut balls {
            ball.speed_y += GRAVITY;
            ball.x += ball.speed_x;
            ball.y += ball.speed_y;

            if ball.x - radius_f <= 0.0 {
                ball.speed_x = ball.speed_x.abs() * WALL_DAMP;
                ball.x = radius_f;
            } else if ball.x + radius_f >= X_MAX {
                ball.speed_x = -ball.speed_x.abs() * WALL_DAMP;
                ball.x = X_MAX - radius_f;
            }

            if ball.y - radius_f <= 0.0 {
                ball.speed_y = ball.speed_y.abs() * WALL_DAMP;
                ball.y = radius_f;
            } else if ball.y + radius_f >= Y_MAX {
                ball.speed_y = -ball.speed_y.abs() * FLOOR_DAMP;
                ball.y = Y_MAX - radius_f;
            }
        }
    }

    let mut lines = vec![
        format!("Ball / 弹球: radius={radius}, balls={num_balls}, trail={trail}"),
        "Host preview: simulated 12 physics frames without TFT hardware.".to_string(),
    ];
    for (index, ball) in balls.iter().enumerate() {
        lines.push(format!(
            "ball #{:02}: pos=({:.1},{:.1}) vel=({:.2},{:.2})",
            index + 1,
            ball.x,
            ball.y,
            ball.speed_x,
            ball.speed_y
        ));
    }
    lines
}

/// 乒乓游戏：host Rust 版本生成一局短回合预览。
pub fn pingpong_game() -> Vec<String> {
    let mut rng = Lcg::seeded();
    let mut x = X_MAX / 2.0;
    let mut y = Y_MAX / 2.0;
    let mut speed_x = if rng.next_u32() % 2 == 0 { 2.0 } else { -2.0 };
    let mut speed_y = if rng.next_u32() % 2 == 0 { 1.5 } else { -1.5 };
    let racket_x = 8.0;
    let racket_y = 92.0;
    let racket_len = 30.0;
    let radius = 5.0;
    let mut score = 0;

    for _ in 0..48 {
        x += speed_x;
        y += speed_y;

        if y - radius <= 0.0 || y + radius >= Y_MAX {
            speed_y = -speed_y;
        }

        if x + radius >= X_MAX {
            speed_x = -speed_x;
        }

        if x - radius <= racket_x + 5.0 && y >= racket_y && y <= racket_y + racket_len {
            x = racket_x + 5.0 + radius;
            speed_x = speed_x.abs();
            score += 1;
        }

        if x - radius < 0.0 {
            break;
        }
    }

    vec![
        "PingPong / 乒乓: host preview".to_string(),
        format!("score={score}, ball=({x:.1},{y:.1}), velocity=({speed_x:.1},{speed_y:.1})"),
        "Use the C++ ESP32 firmware for live Serial controls and TFT animation.".to_string(),
    ]
}

/// D20 骰子游戏：host Rust 版本执行一次二十面骰投掷。
pub fn d20_game() -> Vec<String> {
    let mut rng = Lcg::seeded();
    let value = rng.next_range(1, 20);
    let note = match value {
        20 => "Lucky! / 幸运！",
        1 => "Unlucky :( / 不太走运 :(",
        _ => "Roll complete / 投掷完成",
    };

    vec![
        "D20 / 二十面骰子: roll".to_string(),
        format!("Rolled: {value}"),
        note.to_string(),
    ]
}

/// 抛硬币游戏：host Rust 版本执行一次硬币翻转。
pub fn coin_game() -> Vec<String> {
    let mut rng = Lcg::seeded();
    let heads = rng.next_u32() % 2 == 0;
    let result = if heads {
        "Heads / 正面"
    } else {
        "Tails / 反面"
    };

    vec![
        "Coin / 抛硬币: flip".to_string(),
        format!("You got: {result}"),
    ]
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn ball_preview_contains_requested_count() {
        let lines = ball_game(10, 2, true);
        assert!(lines[0].contains("弹球"));
        assert_eq!(
            lines
                .iter()
                .filter(|line| line.starts_with("ball #"))
                .count(),
            2
        );
    }

    #[test]
    fn chance_games_include_chinese_labels() {
        assert!(pingpong_game()[0].contains("乒乓"));
        assert!(d20_game()[0].contains("二十面骰子"));
        assert!(coin_game()[0].contains("抛硬币"));
    }
}
