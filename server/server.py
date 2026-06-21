from fastapi import FastAPI, HTTPException, Depends
from sqlalchemy import create_engine, Column, Integer, String
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import sessionmaker, Session

from pydantic import BaseModel
from typing import Optional, List

import uvicorn
import random

app = FastAPI()

#database setup
engine = create_engine("sqlite:///cardids.db", connect_args={"check_same_thread":False})
SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)
Base = declarative_base()

#database model
class cardID(Base):
    __tablename__ = "cardIDs"

    id = Column(String, primary_key=True, nullable=False, unique=True)



#pydantic models
class cardIDModel(BaseModel):
    id:str

def get_db():
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()

ids = []
serverVariable = "FD01C906"

@app.get("/hello", response_model=cardIDModel)
def getIP(id: str, db:Session = Depends(get_db)):

    getID = db.query(cardID).filter(cardID.id == id).first()
    if not getID:
        raise HTTPException(status_code=404, detail="Data: not found")
    
    return getID
    #for value in ids:
        #if value == id:
            #return value
    
    #return {"Data: not found"}

@app.get("/hellothere") 
def getIPs():
    return ids

@app.post("/hello")
async def postIP(id: str, db:Session = Depends(get_db)):
    if db.query(cardID).filter(cardID.id == id).first():
        raise HTTPException(status_code=404, detail="Error: Id exists")
    
    newId = cardID(id=id)
    db.add(newId)
    db.commit()
    db.refresh(newId)
    return newId
    #for value in ids:
    #    if value == id:
    #        return {"Error: Id exists "}
        
    #if id == serverVariable:
    #    ids.append(id)
    #    return id
    #else:
    #    raise HTTPException(status_code=404, detail="Error: ID does not match server variable")
  

if __name__ == "__main__":
    Base.metadata.drop_all(bind=engine)   # delete all tables
    Base.metadata.create_all(bind=engine) # recreate empty tables
    uvicorn.run("server:app", host="0.0.0.0", port=8000, reload=True)