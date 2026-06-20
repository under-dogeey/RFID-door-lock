from fastapi import FastAPI, HTTPException
import uvicorn
import random

app = FastAPI()

ids = []
serverVariable = "FD01C906"

@app.get("/hello")
def getIP(id: str):
    for value in ids:
        if value == id:
            return value
    
    return {"Data: not found"}

@app.get("/hellothere") 
def getIPs():
    return ids

@app.post("/hello")
async def postIP(id: str):
    for value in ids:
        if value == id:
            return {"Error: Id exists "}
        
    if id == serverVariable:
        ids.append(id)
        return id
    else:
        raise HTTPException(status_code=404, detail="Error: ID does not match server variable")
  

if __name__ == "__main__":
    uvicorn.run("server:app", host="0.0.0.0", port=8000, reload=True)