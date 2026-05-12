async function loadSpeed () {
    
    try { 
        const response = await fetch ("/api/updatespeed.php", {
        });    
    
    const result =await response.json();
    console.log("Current speed:", result);

    document.querySelector("#speed").textContent = result.speed || "";

}   catch (error) {
    console.error("Failed to get speed:",
    error);
    }

}


loadSpeed();