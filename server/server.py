from fastapi import FastAPI, HTTPException, Depends, Response
from sqlalchemy import create_engine, Column, Integer, String
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import sessionmaker, Session

from pydantic import BaseModel
from typing import Optional, List

import prometheus_client

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

#ids = []
#serverVariable = "FD01C906"

card_count = prometheus_client.Counter(
    "card_count",
    "Number of cards",
)

@app.get("/hello", response_model=cardIDModel)
def getIP(id: str, db:Session = Depends(get_db)):

    newID = db.query(cardID).filter(cardID.id == id).first()
    if not newID:
        raise HTTPException(status_code=404, detail="Data: not found")
    
    return newID

@app.get("/hellothere", response_model=List[cardIDModel]) 
def getIPs(db:Session = Depends(get_db)):
    return db.query(cardID).all()

@app.get('/metrics')
def getMetrics():
    return Response(content=prometheus_client.generate_latest(), media_type="text/plain")

@app.post("/hello")
async def postIP(id: str, add: int, db:Session = Depends(get_db)):
    if(add == 0):
        raise HTTPException(status_code=404, detail="Reader not in add mode")
        
    elif(add == 1):
        if db.query(cardID).filter(cardID.id == id).first():
            raise HTTPException(status_code=404, detail="Error: Id exists")
        
        newID = cardID(id=id)
        db.add(newID)
        db.commit()
        db.refresh(newID)

        card_count.inc() #increment metric
        return newID

    
  

if __name__ == "__main__":
    Base.metadata.drop_all(bind=engine)   # delete all tables
    Base.metadata.create_all(bind=engine) # recreate empty tables
    uvicorn.run(app, host="0.0.0.0", port=8000, reload=False)