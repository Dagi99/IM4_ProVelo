async function loadSpeed () {
    try {    
        const response = await fetch("api/updatespeed.php?t=" + Date.now());
        const result = await response.json();

        if (result.status === "success") {
            console.log("DEBUG - Received from DB:", result);
            document.querySelector("#speed").textContent = result.speed;
        }
    } catch (error) {
        console.error("Error fetching speed:", error);
    }
}
loadSpeed();
setInterval(loadSpeed, 5000);