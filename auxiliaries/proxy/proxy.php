<?php

header("Access-Control-Allow-Origin: *");
header("Access-Control-Allow-Methods: GET, POST, OPTIONS");
header("Access-Control-Allow-Headers: Content-Type");
header("Content-Type: application/json; charset=utf-8");
header('Cache-Control: public, max-age=30');

// ini_set('display_errors', 1); // Enable error display

doTheProxy(false, true);

function doTheProxy($logRequests, $logStats)
{
    // Get the original path and query parameters from the current URL
    $requestUri = $_SERVER['REQUEST_URI'];
    $requestUri = substr($requestUri, strrpos($requestUri, "/api/v3"));

    $expireTime = 10 * 60;
    if (str_contains($requestUri, "/market_chart?")) {
        if (str_contains($requestUri, "&days=60")) {
            $expireTime = 6 * 60 * 60;
        } else {
            $expireTime = 30 * 60;
        }
    } else if (str_contains($requestUri, "coins/markets?")) {
        $expireTime = 1 * 60 * 60;
    } else if (str_contains($requestUri, "search/trending")) {
        $expireTime = 1 * 60 * 60;
    } else if (str_contains($requestUri, "simple/price")) {
        $expireTime = 5 * 60;
    } else if (str_contains($requestUri, "/ping")) {
        header('Cache-Control: public, max-age=1200');
        $expireTime = 24 * 60 * 60;
    } else if (str_contains($requestUri, "/coins/list")) {
        header('Cache-Control: public, max-age=1200');
        $expireTime = 24 * 60 * 60;
    }

    $coinGeckoUrl = 'https://api.coingecko.com';

    // Create a unique identifier for the cache based on the request URL
    $cacheKey = md5($requestUri);

    $status = "unknown";
    $cacheState = isCached($cacheKey, $expireTime);
    if ($cacheState === 0) { // cache hit and valid
        $requestUrl = $requestUri;
        $cachedResult = getCachedResult($cacheKey, $expireTime);
        $response = $cachedResult;
        $status = "cache hit";
    } else { // cache miss or expired
        // Create the full URL for the coingecko server
        $requestUrl = $coinGeckoUrl . $requestUri;
        list($response, $statusCode) = request($requestUrl);

        if ($cacheState === 1) { // cache hit but expired
            if ($statusCode === 429) {
                // Prefer the expired data over 429 response code and refetch
                $cachedResult = getCachedResult($cacheKey, 24 * 60 * 60);
                http_response_code(200);
                $response = $cachedResult;
                $status = "cache old";
            } else {
                if ($statusCode === 200) {
                    saveToCache($cacheKey, $response);
                }
                http_response_code($statusCode);
                $status = "expired -> $statusCode";
            }
        } else if ($cacheState === 2) {// cache miss
            $key = getRandomKey();
            if ($statusCode === 429 && $key !== false) {
                // Create the full URL for the coingecko server
                $requestUrl = $coinGeckoUrl . $requestUri;
                if (strpos($requestUrl, "?") === false) {
                    $requestUrl .= "?";
                } else {
                    $requestUrl .= "&";
                }
                $requestUrl .= "x_cg_demo_api_key=" . $key;
                list($response, $statusCode) = request($requestUrl);
                if ($statusCode === 200) {
                    saveToCache($cacheKey, $response);
                }
                $status = "miss -> 429 -> $statusCode";
            } else {
                if ($statusCode === 200) {
                    saveToCache($cacheKey, $response);
                }
                $status = "miss -> $statusCode";
            }
            http_response_code($statusCode);
        }
    }

    if ($logRequests) {
        file_put_contents("requests.log", timePrefix() . " [$status] " . $_SERVER['REMOTE_ADDR'] . " - " . $requestUrl . "  $cacheKey\n", FILE_APPEND);
    }
    if ($logStats) {
        incStatsValue($status);
    }

    if (http_response_code() !== 200) {
        header('Cache-Control: public, max-age=30');
    }
    echo $response;
}

function request($url)
{
    // Create a stream context with options, including 'http' context
    $context = stream_context_create([
        'http' => [
            'method' => 'GET',
            'header' => 'User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:100.0) Gecko/20100101 Firefox/100.0', // Set a user agent to avoid issues with some servers
        ],
    ]);
    // Make the request and get the content
    $response = file_get_contents($url, false, $context);
    // Get the HTTP status code from the response headers
    list($version, $statusCode, $reasonPhrase) = explode(' ', $http_response_header[0], 3);
    return [$response, (int)$statusCode];
}

function timePrefix()
{
    return "" . time() . " | ";
}

function getRandomKey()
{
    $keysFile = 'keys.json';
    if (file_exists($keysFile)) {
        $jsonContent = file_get_contents($keysFile);
    } else {
        return false;
    }
    $keys = json_decode($jsonContent, true);
    if ($keys === null) {
        return false;
    }
    return $keys[array_rand($keys)];
}

function incStatsValue($key)
{
    $c = getStatsValue($key);
    if ($c !== false) {
        $c++;
    } else {
        $c = 1;
    }
    setStatsValue($key, $c);
}

function setStatsValue($key, $value)
{
    $statsFile = 'stats.json';
    if (file_exists($statsFile)) {
        $jsonData = file_get_contents($statsFile);
        $data = json_decode($jsonData, true);
        $data[$key] = $value;
        ksort($data);
    } else {
        $data = [$key => $value];
    }
    // Convert the data to JSON format
    $jsonData = json_encode($data, JSON_PRETTY_PRINT | JSON_UNESCAPED_SLASHES);
    // Write the JSON data to a file
    file_put_contents($statsFile, $jsonData, LOCK_EX);
}

function getStatsValue($key)
{
    $statsFile = 'stats.json';
    if (file_exists($statsFile)) {
        $jsonData = file_get_contents($statsFile);
        $data = json_decode($jsonData, true);
        if (array_key_exists($key, $data)) {
            return $data[$key];
        }
    }
    return false;
}

// Function to check cache entry
// ret 0: hit and valid
// ret 1: hit but expired
// ret 2: miss
function isCached($cacheKey, $expireTime) {
    $cacheFile = 'cache/' . $cacheKey;
    if (file_exists($cacheFile)) {
        if ((time() - filemtime($cacheFile)) < $expireTime) {
            return 0;
        } else {
            return 1;
        }
    }
    return 2;
}

// Function to retrieve cached result
function getCachedResult($cacheKey, $expireTime) {
    $cacheFile = 'cache/' . $cacheKey;
    if (file_exists($cacheFile)) {
        if ((time() - filemtime($cacheFile)) < $expireTime) {
            return file_get_contents($cacheFile);
        } else {
            // Delete the expired cache file
            unlink($cacheFile);
        }
    }
    return false;
}

// Function to save result to cache
function saveToCache($cacheKey, $data) {
    $cacheDir = 'cache';
    if (!is_dir($cacheDir)) {
        mkdir($cacheDir, 0777, true);
    }
    $cacheFile = $cacheDir . '/' . $cacheKey;
    file_put_contents($cacheFile, $data);
}

?>