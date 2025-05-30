const data = {
    uuid: localStorage.getItem("uuid") || crypto.randomUUID(),
    session_id: sessionStorage.getItem("session_id") || Date.now().toString(),
    user_agent: navigator.userAgent,
    screen: `${screen.width}x${screen.height}`,
    canvas_fingerprint: getCanvasFingerprint(), // From mega_tracker.js
    plugins: getPlugins(), // From s.js
    // ... other fields
};
fetch("http://127.0.0.1:8080/track", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(data),
});