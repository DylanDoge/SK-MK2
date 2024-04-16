let redirect_uri = "http://127.0.0.1:5500/index.html";

let client_id = ""; 
let client_secret = "";

let access_token = null;
let refresh_token = null;

const submitBtn = document.getElementById("authSubmit");
const clearBtn = document.getElementById("clearBtn");
const copyRefresh = document.getElementById("refreshTokenCopy");
const copyAccess = document.getElementById("accessTokenCopy");
const AUTHORIZE = "https://accounts.spotify.com/authorize"
const TOKEN = "https://accounts.spotify.com/api/token";


function onPageLoad(){
    client_id = localStorage.getItem("client_id");
    client_secret = localStorage.getItem("client_secret");
    if ( window.location.search.length > 0 ){
        handleRedirect();
    }
    else{
        access_token = localStorage.getItem("access_token");
        refresh_token = localStorage.getItem("refresh_token");
        let title = document.getElementById("browserTitle");
        if ( access_token == null ){
            // we don't have an access token so present token section
            document.getElementById("tokenSection").style.display = 'flex';
            title.innerText = "Spotify API - Auth";
        }
        else {
            // we have an access token so present device section
            document.getElementById("authInfoSection").style.display = 'flex';
            document.getElementById("refreshToken").innerText = refresh_token;
            document.getElementById("accessToken").innerText = access_token;
            title.innerText = "Spotify API - Keys";
        }
    }
}

function handleRedirect(){
    let code = getCode();
    fetchAccessToken( code );
    window.history.pushState("", "", redirect_uri); // remove param from url
}

function getCode(){
    let code = null;
    const queryString = window.location.search;
    if ( queryString.length > 0 ){
        const urlParams = new URLSearchParams(queryString);
        code = urlParams.get('code')
    }
    return code;
}

function requestAuthorization(){
    client_id = document.getElementById("clientId").value;
    client_secret = document.getElementById("clientSecret").value;
    localStorage.setItem("client_id", client_id);
    localStorage.setItem("client_secret", client_secret); // In a real app you should not expose your client_secret to the user

    let url = AUTHORIZE;
    url += "?client_id=" + client_id;
    url += "&response_type=code";
    url += "&redirect_uri=" + encodeURI(redirect_uri);
    url += "&show_dialog=true";
    url += "&scope=user-read-private user-read-email user-modify-playback-state user-read-playback-position user-library-modify user-library-read streaming user-read-playback-state user-read-recently-played playlist-read-private";
    window.location.href = url; // Show Spotify's authorization screen
}

function fetchAccessToken( code ){
    let body = "grant_type=authorization_code";
    body += "&code=" + code; 
    body += "&redirect_uri=" + encodeURI(redirect_uri);
    body += "&client_id=" + client_id;
    body += "&client_secret=" + client_secret;
    callAuthorizationApi(body);
}

function callAuthorizationApi(body){
    let xhr = new XMLHttpRequest();
    xhr.open("POST", TOKEN, true);
    xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
    xhr.setRequestHeader('Authorization', 'Basic ' + btoa(client_id + ":" + client_secret));
    xhr.send(body);
    xhr.onload = handleAuthorizationResponse;
}

function handleAuthorizationResponse(){
    if ( this.status == 200 ){
        var data = JSON.parse(this.responseText);
        console.log(data);
        var data = JSON.parse(this.responseText);
        if ( data.access_token != undefined ){
            access_token = data.access_token;
            localStorage.setItem("access_token", access_token);
        }
        if ( data.refresh_token  != undefined ){
            refresh_token = data.refresh_token;
            localStorage.setItem("refresh_token", refresh_token);
        }
        onPageLoad();
    }
    else {
        console.log(this.responseText);
        alert(this.responseText);
    }
}

function clearValues()
{
    localStorage.clear();
    client_id = ""; 
    client_secret = "";
    access_token = null;
    refresh_token = null;
    document.getElementById("tokenSection").style.display = 'flex';
    document.getElementById("authInfoSection").style.display = 'none';
    location.reload();
}

function copyToClipboard()
{
    let selected = this.parentElement.parentElement;
    let keyText = selected.childNodes[3];
    let textValues = keyText.innerText;

    keyText.classList.add("copyBlink");
    navigator.clipboard.writeText(textValues);
    setTimeout(() => {
        keyText.classList.remove("copyBlink");
    }, 1000);
}

submitBtn.addEventListener("click", requestAuthorization);
clearBtn.addEventListener("click", clearValues);
copyRefresh.addEventListener("click", copyToClipboard);
copyAccess.addEventListener("click", copyToClipboard);
console.log("elo");