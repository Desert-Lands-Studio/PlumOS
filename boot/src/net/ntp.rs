// src/net/ntp.rs
use std::net::UdpSocket;
use std::time::{SystemTime, UNIX_EPOCH, Duration};

pub fn get_network_time() -> Option<SystemTime> {
    let socket = UdpSocket::bind("0.0.0.0:0").ok()?;
    socket.set_read_timeout(Some(Duration::from_secs(3))).ok()?;
    socket.connect("pool.ntp.org:123").ok()?;

    let mut buf = [0u8; 48];
    buf[0] = 0x1B;

    socket.send(&buf).ok()?;
    socket.recv(&mut buf).ok()?;

    let secs = u32::from_be_bytes([buf[40], buf[41], buf[42], buf[43]]);
    let ntp_time = secs as u64 - 2_208_988_800;

    Some(UNIX_EPOCH + Duration::from_secs(ntp_time))
}
